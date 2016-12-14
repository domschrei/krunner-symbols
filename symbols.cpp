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
    KConfig globalConfig ("/usr/share/config/krunner-symbolsrc");    
    KConfigGroup globalDefGroup( &globalConfig, "Definitions" );
    QMap< QString, QString > globalMap = globalDefGroup.entryMap();
    
    // Local configuration with custom definitions
    KConfig localConfig ("krunner-symbolsrc", KConfig::SimpleConfig);
    KConfigGroup localDefGroup( &localConfig, "Definitions" );
    QMap< QString, QString > localMap = localDefGroup.entryMap();
    
    // Merge the two maps
    globalMap.unite(localMap);
    symbols = globalMap;

    // Unicode symbols
    KConfig unicodeConfig("/usr/share/config/krunner-symbols-unicode-index");
    KConfigGroup unicodeGroup(&unicodeConfig, "Unicode");
    unicodeSymbols = unicodeGroup.entryMap();
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
                // the query is a (non-complete) prefix of the keyword -> completion match
                match.setType(Plasma::QueryMatch::CompletionMatch);
            }
            // also show the exact keyword for this value
            match.setText(it.value());
            match.setSubtext("[" + foundKey + "]");
            
            // Check if the result is a command ("open:" or "exec:")
            if (it.value().startsWith("open:"))
            {
                match.setText(match.text().replace("open:", "→ "));
            } else if (it.value().startsWith("exec:")) {
                match.setText(match.text().replace("exec:", ">_ "));
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
    
    // Also look for fitting unicode symbols
    matchUnicode(context);
}

void Symbols::matchUnicode(Plasma::RunnerContext &context)
{
    if (!context.isValid()) return;
    const QString enteredKey = context.query();
    QStringList enteredTokens = enteredKey.split(' ', QString::SkipEmptyParts);
    
    QList<Plasma::QueryMatch> matches;
    QMapIterator<QString, QString> it(unicodeSymbols);
    int amountUnicodeMatches = 0;
    
    while (it.hasNext()) {
        
        it.next();
        QString foundKey = it.key();
        
        bool isMatching = false;
        QListIterator<QString> tokenIt(enteredTokens);
        while (tokenIt.hasNext()) {
            QString token = tokenIt.next();
            isMatching |= foundKey.contains(token, Qt::CaseInsensitive);
            if (isMatching) {
                break;
            }
        }
        
        if (isMatching) {
            //const QString str = QLatin1String(it.value());
            bool ok;
            const unsigned int parsedValue = it.value().toUInt(&ok, 16);
            if (ok) {
            
                Plasma::QueryMatch match(this);
                QString result = QChar(parsedValue);
                
                std::cout << "match with " << result.toStdString() << std::endl;
                
                match.setType(Plasma::QueryMatch::CompletionMatch);
                match.setText(result);
                match.setSubtext("[" + foundKey + "]");
                
                // Basic properties for the match
                match.setIcon(QIcon::fromTheme("preferences-desktop-font"));
                //match.setSubtext(result);
                
                // The match's relevance gets higher the more "complete" the query string is
                // (k/x for query length k and keyword length x; 1 for complete keyword)
                match.setRelevance(0.5f);
                
                matches.append(match);   
                amountUnicodeMatches++;
                
                /*if (amountUnicodeMatches >= maxUnicodeMatches) {
                    std::cout << "not showing " << result.toStdString() << std::endl;
                    break;
                }*/
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

    if (match.text().startsWith("→ "))
    {
        // Open a file or a URL in a (file/web) browser
        // (in a new process, so that krunner doesn't get stuck while opening the path)
        string command = "kde-open " + match.text().remove("→ ").toStdString() + " &";
        system(command.c_str());
        
    } else if (match.text().startsWith(">_ ")) 
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
