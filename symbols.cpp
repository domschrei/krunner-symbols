/******************************************************************************
 *  Copyright (C) 2016 by Dominik Schreiber <dev@dominikschreiber.de>         *
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
#include <KConfigCore/KConfig>
#include <KConfigCore/KConfigGroup>

#include <QApplication>
#include <QClipboard>
#include <QRegExp>
#include <QTextCodec>

#include "symbols.h"

using namespace std;

Symbols::Symbols(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);
    
    // General runner configuration
    setObjectName(QLatin1String("Symbols"));
    setHasRunOptions(true);
    setIgnoredTypes(Plasma::RunnerContext::Directory |
                    Plasma::RunnerContext::File |
                    Plasma::RunnerContext::NetworkLocation);
    setSpeed(AbstractRunner::NormalSpeed);
    setPriority(HighestPriority);
    setDefaultSyntax(
        Plasma::RunnerSyntax(
            QString::fromLatin1(":q:"),
            i18n("Looks for a unicode symbol described by :q: and, if present, displays it. Then pressing ENTER copies the symbol to the clipboard.")
        )
    );

    loadConfig();
}

Symbols::~Symbols() {}

void Symbols::loadConfig() {
    
    // Global configuration (meant to be immutable by user)
    KConfig globalConfig("/usr/share/config/krunner-symbolsrc");    
    // Local configuration with custom definitions and preferences
    KConfig localConfig("krunner-symbolsrc", KConfig::SimpleConfig);
    
    // Symbol definitions
    KConfigGroup globalDefGroup(&globalConfig, "Definitions");    
    QMap<QString, QString> globalDefMap = globalDefGroup.entryMap();
    KConfigGroup localDefGroup(&localConfig, "Definitions");
    QMap<QString, QString> localDefMap = localDefGroup.entryMap();
    mergeMapsOverriding(&globalDefMap, &localDefMap);
    symbols = localDefMap;
    
    // Find and change definitions of the form "a,b,c=d"
    // to "a=d", "b=d", "c=d"
    expandMultiDefinitions();
    
    // Preferences configuration
    KConfigGroup globalPrefGroup(&globalConfig, "Preferences");
    QMap<QString, QString> globalPrefMap = globalPrefGroup.entryMap();
    KConfigGroup localPrefGroup(&localConfig, "Preferences");
    QMap<QString, QString> localPrefMap = localPrefGroup.entryMap();
    mergeMapsOverriding(&globalPrefMap, &localPrefMap);
    QMap<QString, QString> prefMap = localPrefMap;
    
    // UseUnicodeDatabase: Default true
    prefs.insert("UseUnicodeDatabase", 
                 (!prefMap.contains("UseUnicodeDatabase")) 
                 || prefMap.value("UseUnicodeDatabase").compare("true") == 0);
    
    // Unicode symbols (only if not disabled by preferences)
    if (prefs.value("UseUnicodeDatabase").toBool()) {
        KConfig unicodeConfig("/usr/share/config/krunner-symbols-full-unicode-index");
        KConfigGroup unicodeGroup(&unicodeConfig, "Unicode");
        unicodeSymbols = unicodeGroup.entryMap();
    }
}

void Symbols::match(Plasma::RunnerContext &context)
{
    if (!context.isValid()) return;

    const QString enteredKey = context.query();
    
    QList<Plasma::QueryMatch> matches;
    QMapIterator<QString, QString> it(symbols);
    
    // Add matches for every keyword that equals the query
    // or that the query could be completed to
    while (it.hasNext()) {
      
        it.next();
        QString foundKey = it.key();
        
        if (foundKey.startsWith(enteredKey))
        {
            // We have a match
            Plasma::QueryMatch match(this);
        
            if (foundKey.length() == enteredKey.length()) {
                // the query equals the keyword -> exact match
                match.setType(Plasma::QueryMatch::ExactMatch);
            } else {
                // the query is a (non-complete) prefix of the keyword 
                // -> completion match
                match.setType(Plasma::QueryMatch::CompletionMatch);
            }
            // also show the exact keyword for this value
            match.setText(it.value());
            match.setSubtext("[" + foundKey + "]");
            
            // Check if the result is a command ("open:" or "exec:")
            if (it.value().startsWith("open:"))
            {
                match.setText(match.text().replace("open:", "→ "));
                match.setData("open");
            } else if (it.value().startsWith("exec:")) {
                match.setText(match.text().replace("exec:", ">_ "));
                match.setData("execute");
            } else {
                match.setData("symbol");
            }
            
            // Basic properties for the match
            match.setIcon(QIcon::fromTheme("preferences-desktop-font"));
            //match.setSubtext(it.value());
            
            // The match's relevance gets higher the more "complete" the query string is
            // (k/x for query length k and keyword length x; 1 for complete keyword)
            match.setRelevance((float) enteredKey.length() / (float) foundKey.length());
            
            matches.append(match);
        }
    }   

    // Feed the framework with the calculated results
    context.addMatches(matches);

    if (prefs.value("UseUnicodeDatabase").toBool()) {
        // Also look for fitting unicode symbols (only if not disabled by preferences)
        matchUnicode(context);
    }
}

void Symbols::matchUnicode(Plasma::RunnerContext &context)
{
    if (!context.isValid()) return;
    const QString enteredKey = context.query();
    
    // do not match against searches with less than three characters
    if (enteredKey.length() < 3) {
        return;
    }
    
    QStringList enteredTokens = enteredKey.split(' ', QString::SkipEmptyParts);
    QList<Plasma::QueryMatch> matches;
    
    // Iterate over all available unicode symbols
    QMapIterator<QString, QString> it(unicodeSymbols);
    while (it.hasNext()) {
       
        it.next();
        QString foundKey = it.key(); // symbol description
     
        // actual relevance and maximum relevance of this match
        float relevance = 0.0f;
        float maxRelevance = 0.0f;
            
        // Now iterate over the entered search tokens
        QListIterator<QString> tokenIt(enteredTokens);
        while (tokenIt.hasNext()) {
            QString enteredToken = tokenIt.next();
            bool someMatch = false;
    
            // Iterate over all tokens of the unicode symbol description
            QListIterator<QString> unicodeTokens(foundKey.split(' ', QString::SkipEmptyParts));
            while (unicodeTokens.hasNext()) {
                QString unicodeToken = unicodeTokens.next();
                        
                // Depending on the following comparisons,
                // some relevance heuristic for the found token is being applied
                if (unicodeToken.compare(enteredToken, Qt::CaseInsensitive) == 0) {
                    // exact match between entered token and found token
                    relevance += 1.0f * enteredToken.length();
                    someMatch = true;
                } else if (unicodeToken.startsWith(enteredToken, Qt::CaseInsensitive)) {
                    // found token begins with entered token
                    relevance += 0.5f * enteredToken.length();
                    someMatch = true;
                } else if (unicodeToken.contains(enteredToken, Qt::CaseInsensitive)) {
                    // found token contains entered token
                    relevance += 0.25f * enteredToken.length();
                }
            }
            
            maxRelevance += enteredToken.length();
            
            // Punish if a keyword is not contained in the found description at all
            if (!someMatch) {
                relevance -= enteredToken.length();
            }
        }
        
        // Shorter unicode matches should be preferred 
        // over longer unicode matches with the same relevance
        relevance *= enteredKey.length() / ((float) foundKey.length());
        
        // Some relevance means that there is a (partial) match
        if (relevance > 0) {
            
            // Try to transform the value of the found object into a unicode symbol
            bool ok;
            QString resultStr = it.value();
            unsigned int parsedValue = resultStr.toUInt(&ok, 16);
            
            if (ok) {
                QString result = QChar(parsedValue);
                relevance = (relevance / maxRelevance);
                
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
                Plasma::QueryMatch match(this);
                // decide whether this is an exact match or just a possible match
                if (relevance >= 1.0f - 0.0001) {
                    match.setType(Plasma::QueryMatch::ExactMatch);
                } else {
                    match.setType(Plasma::QueryMatch::PossibleMatch);
                }
                
                match.setText(result);
                match.setSubtext("[" + foundKey + "]");
                match.setData("symbol");
                match.setIcon(QIcon::fromTheme("preferences-desktop-font"));
                match.setRelevance(relevance);
                
                matches.append(match);  
            }
        }
    }
    
    // Feed the framework with the calculated results
    context.addMatches(matches);
}

/**
 * Perform an action when a user chooses one of the previously found matches.
 * Either some string gets copied to the clipboard, a file/path/URL is being opened, 
 * or a command is being executed.
 */
void Symbols::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context);

    if (match.data().toString().compare("open") == 0)
    {
        // Open a file or a URL in a (file/web) browser
        // (in a new process, so that krunner doesn't get stuck while opening the path)
        string command = "kde-open " + match.text().remove("→ ").toStdString() + " &";
        system(command.c_str());
        
    } else if (match.data().toString().compare("execute") == 0) 
    {
        // Execute a command
        // (in a new process, so that krunner doesn't get stuck while opening the path)
        string command = match.text().remove(">_ ").toStdString() + " &";
        system(command.c_str());
        
    } else 
    {
        // Copy the result to clipboard
        QApplication::clipboard()->setText(match.text());
    }
}

/*
 * Stores the union of two QMap<QString,QString> objects inside the second map, 
 * whereas an entry of the overriding map will be preferred (and the overridden map's 
 * one will be ignored) in case of duplicate keys.
 */
void Symbols::mergeMapsOverriding(QMap<QString, QString> *overriddenMap, QMap<QString, QString> *overridingMap) {
    QMapIterator<QString,QString> it(*overriddenMap);
    while (it.hasNext()) {
        it.next();
        if (!overridingMap->contains(it.key())) {
            overridingMap->insert(it.key(), it.value());
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

K_EXPORT_PLASMA_RUNNER(symbols, Symbols)

#include "symbols.moc"
