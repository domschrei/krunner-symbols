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

#include "symbols.h"

#include <KLocalizedString>
#include <QApplication>
#include <QClipboard>
#include <KConfigCore/KConfig>
#include <KConfigCore/KConfigGroup>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <iostream>

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

    // Global configuration (meant to be immutable by user)
    KConfig globalConfig("/usr/share/config/krunner-symbolsrc");    
    KConfigGroup globalDefGroup(&globalConfig, "Definitions");
    QMap<QString, QString> globalMap = globalDefGroup.entryMap();
    
    // Local configuration with custom definitions
    KConfig localConfig("krunner-symbolsrc", KConfig::SimpleConfig);
    
    // Preferences inside local configuration
    KConfigGroup preferences(&localConfig, "Preferences");
    // UseUnicodeDatabase: Default true
    prefs.insert("UseUnicodeDatabase", 
                 (!preferences.entryMap().contains("UseUnicodeDatabase")) 
                 || preferences.entryMap().value("UseUnicodeDatabase").compare("true") == 0);
    
    // Custom definitions inside local configuration
    KConfigGroup localDefGroup(&localConfig, "Definitions");
    QMap<QString, QString> localMap = localDefGroup.entryMap();
    
    // Merge the two "Definitions" maps
    globalMap.unite(localMap);
    symbols = globalMap;

    // Unicode symbols (only if not disabled by preferences)
    if (prefs.value("UseUnicodeDatabase").toBool()) {
        KConfig unicodeConfig("/usr/share/config/krunner-symbols-full-unicode-index");
        KConfigGroup unicodeGroup(&unicodeConfig, "Unicode");
        unicodeSymbols = unicodeGroup.entryMap();
    }
}

Symbols::~Symbols()
{
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
        // is being calculated on-the-fly
        float relevance = 0.0f;
        float maxRelevance = 0.0f;
    
        // Iterate over all tokens of the unicode symbol description
        QListIterator<QString> unicodeTokens(foundKey.split(' ', QString::SkipEmptyParts));
        while (unicodeTokens.hasNext()) {
            QString unicodeToken = unicodeTokens.next();
            
            float newRelevance = 0.0f;
            
            // Now iterate over the entered search tokens
            QListIterator<QString> tokenIt(enteredTokens);
            while (tokenIt.hasNext()) {
                QString enteredToken = tokenIt.next();
        
                // Depending on the following comparisons,
                // some relevance heuristic for the found token is being applied
                // (Only the best match for the found token matters to the final relevance)
                if (unicodeToken.compare(enteredToken, Qt::CaseInsensitive) == 0) {
                    // exact match between entered token and found token
                    newRelevance = std::max(newRelevance, 1.0f * enteredToken.length());
                } else if (unicodeToken.startsWith(enteredToken, Qt::CaseInsensitive)) {
                    // found token begins with entered token
                    newRelevance = std::max(newRelevance, 0.75f * enteredToken.length());
                } else if (unicodeToken.contains(enteredToken, Qt::CaseInsensitive)) {
                    // found token contains entered token
                    newRelevance = std::max(newRelevance, 0.5f * enteredToken.length());
                }
            }
            
            relevance += newRelevance;
            maxRelevance += unicodeToken.length();
        }
        
        // Some relevance means that there is a (partial) match
        if (relevance > 0) {
            
            // Try to transform the value of the found object into a unicode symbol
            bool ok;
            const unsigned int parsedValue = it.value().toUInt(&ok, 16);
            if (ok) {

                QString result = QChar(parsedValue);
                relevance = (relevance / maxRelevance);
                
                // create match object
                Plasma::QueryMatch match(this);
                // decide whether this is an exact match or just a possible match
                if (relevance >= 1.0f - 0.0001) {
                    // if there is an exact match and the search term is longer
                    // than nessessary, the relevance will be decreased
                    relevance *= ((float) foundKey.length()) / enteredKey.length();
                    if (relevance >= 1.0f - 0.0001) {
                        match.setType(Plasma::QueryMatch::ExactMatch);
                    } else {
                        match.setType(Plasma::QueryMatch::PossibleMatch);
                    }
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

K_EXPORT_PLASMA_RUNNER(symbols, Symbols)

#include "symbols.moc"
