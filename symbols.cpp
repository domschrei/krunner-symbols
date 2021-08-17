/******************************************************************************
 *  Copyright (C) 2016-2020 by Dominik Schreiber <dev@dominikschreiber.de>    *
 *                                                                            *
 *  This library is free software; you can redistribute it and/or modify      *
 *  it under the terms of the GNU General Public License as published         *
 *  by the Free Software Foundation; either version 3 of the License or (at   *
 *  your option) any later version.                                           *
 *                                                                            *
 *  This library is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *  Library General Public License for more details.                          *
 *                                                                            *
 *  You should have received a copy of the GNU General Public License         *
 *  along with this library; see the file LICENSE.                            *
 *  If not, see <http://www.gnu.org/licenses/>.                               *
 *****************************************************************************/

#include <iostream>

#include <KLocalizedString>

#include <QApplication>
#include <QClipboard>
#include <QRegExp>
#include <QTextCodec>
#include <QLocale>

#include "symbols.h"

using namespace std;

Symbols::Symbols(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args), 
    localConfig("krunner-symbolsrc", KConfig::SimpleConfig)
{
    // General runner configuration
    setObjectName(QLatin1String("Symbols"));
    setPriority(HighestPriority);
    addSyntax(
        Plasma::RunnerSyntax(
            QString::fromLatin1(":q:"),
            i18n("Looks for a unicode symbol described by :q: and, if present, displays it. Then pressing ENTER copies the symbol to the clipboard.")
        )
    );

    loadConfig();
}

void Symbols::loadConfig() {
    
    // Global configuration (meant to be immutable by user)
    KConfig globalConfig("/usr/share/config/krunner-symbolsrc");
    
    // Symbol definitions
    KConfigGroup globalDefGroup(&globalConfig, "Definitions");    
    QMap<QString, QString> globalDefMap = globalDefGroup.entryMap();
    KConfigGroup localDefGroup(&localConfig, "Definitions");
    QMap<QString, QString> localDefMap = localDefGroup.entryMap();
    mergeMapsOverriding(globalDefMap, localDefMap);
    symbols = localDefMap;
    
    // Find and change definitions of the form "a,b,c=d"
    // to "a=d", "b=d", "c=d"
    expandMultiDefinitions();
    
    // Preferences configuration
    KConfigGroup globalPrefGroup(&globalConfig, "Preferences");
    QMap<QString, QString> globalPrefMap = globalPrefGroup.entryMap();
    KConfigGroup localPrefGroup(&localConfig, "Preferences");
    QMap<QString, QString> localPrefMap = localPrefGroup.entryMap();
    mergeMapsOverriding(globalPrefMap, localPrefMap);
    QMap<QString, QString> prefMap = localPrefMap;
    
    // UseUnicodeDatabase: Default true
    prefs.insert("UseUnicodeDatabase", 
                 (!prefMap.contains("UseUnicodeDatabase")) 
                 || prefMap.value("UseUnicodeDatabase").compare("true") == 0);
    
    // UseLocalizedUnicodeDescriptions: Default true
    prefs.insert("UseLocalizedUnicodeDescriptions",
                 (!prefMap.contains("UseLocalizedUnicodeDescriptions"))
                 || prefMap.value("UseLocalizedUnicodeDescriptions").compare("true") == 0);
    
    // UseInlineDefinitionEditing: Default false
    prefs.insert("UseInlineDefinitionEditing",
                 (prefMap.contains("UseInlineDefinitionEditing"))
                 && prefMap.value("UseInlineDefinitionEditing").compare("true") == 0);
    
    // Unicode symbols (only if not disabled by preferences)
    if (!prefs.value("UseUnicodeDatabase").toBool()) return; 
        
    QString unicodeConfigBase = "/usr/share/config/krunner-symbols-unicode/unicode_";
    bool useLocalized = false;
    if (prefs.value("UseLocalizedUnicodeDescriptions").toBool()) {
        
        // Get locale (only the part left of the underscore)
        QString locale = globalConfig.locale();
        int idxOfUnderscore = locale.indexOf("_");
        if (idxOfUnderscore >= 0) {
            locale = locale.left(idxOfUnderscore);
        }

        // Try to get Unicode symbols file for that locale
        KConfig unicodeConfig(unicodeConfigBase + locale);
        KConfigGroup unicodeGroup(&unicodeConfig, "Unicode");
        if (unicodeGroup.exists()) {
            useLocalized = true;
            unicodeSymbols = unicodeGroup.entryMap();
        }
    }

    if (!useLocalized) {
        // Use default (english) Unicode symbols file
        KConfig defaultUnicodeConfig(unicodeConfigBase + "en");
        KConfigGroup unicodeGroup = KConfigGroup(&defaultUnicodeConfig, "Unicode");
        if (!unicodeGroup.exists()) return;
        unicodeSymbols = unicodeGroup.entryMap();
    }
}

void Symbols::match(Plasma::RunnerContext &context) {
    if (!context.isValid()) return;

    if (prefs.value("UseInlineDefinitionEditing").toBool() 
        && context.query().startsWith("sym:", Qt::CaseInsensitive)) {
        
        // Add or modify a definition
        QRegExp rx("\\bsym:def (.*)=(.*)\\b");
        if (rx.indexIn(context.query()) >= 0) {
            // (Re)define a symbol
            QString key = rx.cap(1).trimmed();
            QString val = rx.cap(2).trimmed();
            Plasma::QueryMatch match;
            QStringList data;
            match = getMatchObject("Define symbol", "[" + key + "=" + val + "]", 1.0f);
            data << "def" << key << val;
            match.setData(data);
            context.addMatch(match);                        
        } else {
            // Remove a definition
            rx = QRegExp("\\bsym:rm (.*)\\b");
            if (rx.indexIn(context.query()) >= 0 && context.query().indexOf("=") < 0) {
                QString key = rx.cap(1).trimmed();
                
                // Check if the definition exists
                auto localDefs = KConfigGroup(&localConfig, "Definitions");
                if (!localDefs.hasKey(key)) return;
                
                Plasma::QueryMatch match;
                QStringList data;
                match = getMatchObject("Remove symbol", "[" + key + "]", 1.0f);
                data << "rm" << key;
                match.setData(data);
                context.addMatch(match);
            }
        }

    } else {
        // Match against symbols from (global & local) configuration
        matchSymbols(context);

        if (prefs.value("UseUnicodeDatabase").toBool()) {
            // Also look for fitting unicode symbols (only if not disabled by preferences)
            matchUnicode(context);
        }
    }
}

void Symbols::matchSymbols(Plasma::RunnerContext &context) {

    const QString enteredKey = context.query();

    // Split query into tokens, exact (in quotations) and inexact,
    // and get maximum relevance number any match could possibly have
    int inputLength = 0;
    auto pair = getExactAndInexactTokens(enteredKey, inputLength);
    std::vector<QString> exactEnteredTokens = pair.first;
    std::vector<QString> inexactEnteredTokens = pair.second;

    QList<Plasma::QueryMatch> matches;    
    
    // Search symbols from database
    QMapIterator<QString, QString> it(symbols);
    while (it.hasNext()) {
        it.next();
        QString foundKey = it.key();
        
        // Check relevance of key
        float relevance = getRelevance(exactEnteredTokens, inexactEnteredTokens, inputLength, foundKey);
        if (relevance > 0) {

            // We have a match
            Plasma::QueryMatch match = getMatchObject(it.value(), "[" + foundKey + "]", relevance);
        
            // Check if the result is a command ("open:" or "exec:")
            if (it.value().startsWith("open:")) {
                match.setText(match.text().replace("open:", "→ "));
                match.setData("open");
            } else if (it.value().startsWith("exec:")) {
                match.setText(match.text().replace("exec:", ">_ "));
                match.setData("execute");
            } else {
                match.setData("symbol");
            }
            
            matches.append(match);
        }
    }   

    // Feed the framework with the calculated results
    context.addMatches(matches);
}

void Symbols::matchUnicode(Plasma::RunnerContext &context) {
    
    const QString enteredKey = context.query();
    
    // do not match against too brief or too large searches
    if (enteredKey.length() < 3 || enteredKey.length() > 100) {
        return;
    }
    
    // Split query into tokens, exact (in quotations) and inexact,
    // and get maximum relevance number any match could possibly have
    int inputLength = 0;
    auto pair = getExactAndInexactTokens(enteredKey, inputLength);
    std::vector<QString> exactEnteredTokens = pair.first;
    std::vector<QString> inexactEnteredTokens = pair.second;

    QList<Plasma::QueryMatch> matches;
    
    // Iterate over all available unicode symbols
    QMapIterator<QString, QString> it(unicodeSymbols);
    while (it.hasNext()) {
       
        it.next();
        const QString& foundKey = it.key(); // symbol description
     
        // Calculate relevance of this match
        float relevance = getRelevance(exactEnteredTokens, inexactEnteredTokens, inputLength, foundKey);
        
        // Some relevance means that there is a (partial) match
        if (relevance > 0) {
             
            // Try to transform the value of the found object into a unicode symbol
            bool ok;
            QString resultStr = it.value();
            unsigned int parsedValue = resultStr.toUInt(&ok, 16);
            
            if (ok) {
                QString result = QChar(parsedValue);
                relevance = (relevance / inputLength);
                
                // Special treatment for symbols longer than 4 hex characters
                if (resultStr.length() > 4) {
                    while (resultStr.length() % 4 != 0) {
                        resultStr = "0" + resultStr;
                    }
                    QByteArray arr = QByteArray::fromHex(qPrintable(resultStr));
                    QTextCodec *codec = QTextCodec::codecForName("UTF-32BE");
                    result = codec->toUnicode(arr);
                }
                
                // create match object
                Plasma::QueryMatch match = getMatchObject(result, "[" + foundKey + "]", relevance);
                match.setData("symbol");
                matches.append(match);  
            }
        }
    }
    
    // Feed the framework with the calculated results
    context.addMatches(matches);
}

std::pair<std::vector<QString>, std::vector<QString>> Symbols::getExactAndInexactTokens(
        const QString& entered, 
        int& inputLength) {

    std::vector<QString> exactEnteredTokens;
    std::vector<QString> inexactEnteredTokens;
    QString exactToken = "";
    QString inexactToken = "";
    bool exact = false;
    inputLength = 0;
    for (int pos = 0; pos < entered.length(); pos++) {
        const QChar c = entered[pos];
        if (c == '"') {
            // Opening or closing quotation
            if (exact) {
                // Closing quotation
                exactEnteredTokens.push_back(exactToken);
                exactToken = "";
                exact = false;
            } else exact = true;
        } else if (c == ' ' && !exact) {
            // space outside exact token
            if (inexactToken.length() > 0) {
                inexactEnteredTokens.push_back(inexactToken);
                inexactToken = "";
            }
        } else {
            if (exact) {
                exactToken += c;
            } else {
                inexactToken += c;
            }
            inputLength++;
        }
    }
    if (exactToken.length() > 0) exactEnteredTokens.push_back(exactToken);
    if (inexactToken.length() > 0) inexactEnteredTokens.push_back(inexactToken);

    return std::pair<std::vector<QString>, std::vector<QString>>(exactEnteredTokens, inexactEnteredTokens);
}

float Symbols::getRelevance(const std::vector<QString>& enteredExact,
                       const std::vector<QString>& enteredInexact, 
                       int inputLength, const QString& found) {

    float relevance = 0;

    // Iterate over the entered inexact (normal) entered search tokens
    for (const QString& enteredToken : enteredInexact) {
        bool someMatch = false;

        // Iterate over all tokens of the found symbol description
        QListIterator<QString> unicodeTokens(found.split(' ', QString::SkipEmptyParts));
        while (unicodeTokens.hasNext()) {
            const QString& foundToken = unicodeTokens.next();

            // Depending on the following comparisons,
            // some relevance for the found token is added
            if (foundToken.compare(enteredToken, Qt::CaseInsensitive) == 0) {
                // exact match between entered token and found token
                relevance += 1.0f * enteredToken.length();
                someMatch = true;
            } else if (foundToken.startsWith(enteredToken, Qt::CaseInsensitive)) {
                // found token begins with entered token
                relevance += 0.5f * enteredToken.length();
                someMatch = true;
            } else if (foundToken.contains(enteredToken, Qt::CaseInsensitive)) {
                // found token contains entered token
                relevance += 0.25f * enteredToken.length();
                someMatch = true;
            }
        }
        
        // Punish if a keyword is not contained in the found description at all
        if (!someMatch) {
            relevance -= 2 * enteredToken.length();
        }
    }
    
    // Iterate over the exact search tokens
    for (const QString& enteredToken : enteredExact) {
        if (found.contains(enteredToken, Qt::CaseInsensitive)) {
            // Entered token exactly matches (partial) symbol description
            relevance += enteredToken.length();
        } else {
            // No exact match: drop found key
            relevance = 0;
            break;
        }
    }

    // Shorter unicode matches should be preferred 
    // over longer unicode matches with the same relevance
    return relevance * inputLength / found.length();
}

Plasma::QueryMatch Symbols::getMatchObject(const QString& text, const QString& subtext, float relevance) {

    Plasma::QueryMatch match(this);

    // decide whether this is an exact match or just a possible match
    match.setType(relevance >= 0.999f ? Plasma::QueryMatch::ExactMatch : Plasma::QueryMatch::PossibleMatch);
    
    match.setText(text);
    if (subtext.length() > 0) match.setSubtext(subtext);
    match.setIcon(QIcon::fromTheme("preferences-desktop-font"));
    match.setRelevance(relevance);

    return match;
}

/**
 * Perform an action when a user chooses one of the previously found matches.
 * Either some string is copied to the clipboard, a file/path/URL is opened, 
 * or a command is executed.
 */
void Symbols::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) {
    Q_UNUSED(context);
// Otherwise compiler shows warning that the results of the system() call is ignored
#pragma GCC diagnostic ignored "-Wunused-result"

    if (match.data().canConvert<QStringList>() && match.data().toStringList().size() >= 2) {
        // Inline addition, modification, or removal of a definition
        QStringList list = match.data().toStringList();
        QString mode = list[0];
        QString key = list[1];
        auto localDefs = KConfigGroup(&localConfig, "Definitions");
        if (QString::compare(mode, "def") == 0) {
            // Add a new definition
            QString val = list[2];
            localDefs.writeEntry(key, val);
            localDefs.sync();
            // Reload config
            loadConfig();
        } else if (QString::compare(mode, "rm") == 0) {
             // Remove a definition
            localDefs.deleteEntry(key);
            localDefs.sync();
            // Reload config
            loadConfig();
        }

    } else if (match.data().toString().compare("open") == 0) {
        // Open a file or a URL in a (file/web) browser
        // (in a new process, so that krunner doesn't get stuck while opening the path)
        string command = "kde-open5 " + match.text().remove("→ ").toStdString() + " &";
        system(command.c_str());
        
    } else if (match.data().toString().compare("execute") == 0) {
        // Execute a command
        // (in a new process, so that krunner doesn't get stuck while opening the path)
        string command = match.text().remove(">_ ").toStdString() + " &";
        system(command.c_str());

    } else {
        // Copy the result to clipboard
        QApplication::clipboard()->setText(match.text());
    }
#pragma GCC diagnostic pop
}

/*
 * Stores the union of two QMap<QString,QString> objects inside the second map, 
 * whereas an entry of the overriding map will be preferred (and the overridden map's 
 * one will be ignored) in case of duplicate keys.
 */
void Symbols::mergeMapsOverriding(QMap<QString, QString>& overriddenMap, QMap<QString, QString>& overridingMap) {
    QMapIterator<QString,QString> it(overriddenMap);
    while (it.hasNext()) {
        it.next();
        if (!overridingMap.contains(it.key())) {
            overridingMap.insert(it.key(), it.value());
        }
    }
}

/*
 * Iterates over all symbol definitions and looks for definitions of the kind
 * "a,b,c=d". These definitions are expanded to seperate definitions of the 
 * form "a=d", "b=d", "c=d".
 */
void Symbols::expandMultiDefinitions() {
    
    QMap<QString, QString> splittedSymbols;
    QStringList keysToRemove; // expanded keys are stored herein
    QMapIterator<QString, QString> it(symbols);
    QRegExp exp("^(.+,)+.+$"); // regex for multiple keys
    
    // For each known symbol
    while (it.hasNext()) {
        it.next();
        QString key = it.key();
        int pos = exp.indexIn(key, 0);
        if (pos >= 0) {
            
            // Multi definition found
            QStringList keywords = key.split(",");
            QStringListIterator strIt(keywords);
            while (strIt.hasNext()) {
                
                // Add a new symbol entry for each part of the key
                QString newKey = strIt.next();
                splittedSymbols.insert(newKey, it.value());
            }
            // The old key is to be deleted
            keysToRemove.append(key);
        }
    }
    // Remove old keys
    QStringListIterator strIt(keysToRemove);
    while (strIt.hasNext()) {
        symbols.remove(strIt.next());
    }
    
    // Merge the ney symbols into the map with all symbols
    symbols.unite(splittedSymbols);
}

K_EXPORT_PLASMA_RUNNER_WITH_JSON(Symbols, "symbols.json")

#include "symbols.moc"
