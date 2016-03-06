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
#include <iostream>
#include <fstream>
#include <regex>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

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
    
    // Get the user's home directory
    const char* homedir;
    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }
    string home = homedir;
    
    // Read the standard config file
    readFile(home + "/.config/krunner-symbols");
    // If present, read the additional, overriding user config file
    readFile(home + "/.config/krunner-symbols-custom");
}

Symbols::~Symbols()
{
}

void Symbols::readFile(string filepath)
{
    string line;
    ifstream file (filepath);
    if (file.is_open())
    {
        while ( getline (file,line) )
        {
            // skip commentaries (lines beginning with # apart from whitespace)
            if (!std::regex_match (line, std::regex("(\s*)(#)(.*)")))
            {
                // only use lines containing an "equals" sign
                // with at least a character before and after
                if (std::regex_match (line, std::regex("(.*)(.)(=)(.)(.*)")))
                {
                    int index = line.find("=", 0);
                    string key = line.substr(0, index);
                    string value = line.substr(index + 1);
                    symbols[key] = value;
                    keylist.push_back(key);
                }
            }
        }
        file.close();
    }
    else cout << "Unable to open file " + filepath + ".";
}

void Symbols::match(Plasma::RunnerContext &context)
{
    if (!context.isValid()) return;
    
    // Convert query to std::string
    const QString term = context.query();
    string enteredKey = term.toStdString();
    
    // Add matches for every keyword that equals the query
    // or that the query could be completed to
    QList<Plasma::QueryMatch> matches;
    for (int i = 0; i < keylist.size(); i++)
    {
        const string key = keylist[i];
        
        // The query must not be longer than the keyword;
        // The query must be a prefix of the keyword
        if (key.length() >= enteredKey.length() && key.substr(0, enteredKey.length()).compare(enteredKey) == 0)
        {
            // We have a match
            Plasma::QueryMatch match(this);
            
            if (key.length() == enteredKey.length())
                // the query equals the keyword -> exact match
                match.setType(Plasma::QueryMatch::ExactMatch);
            else
                // the query is a (non-complete) prefix of the keyword -> completion match
                match.setType(Plasma::QueryMatch::CompletionMatch);
            
            // Basic properties for the match
            match.setIcon(QIcon::fromTheme("preferences-desktop-font"));
            string result = symbols[key];
            match.setText(QString::fromStdString(result));
            
            // The match's relevance gets higher the more "complete" the query string is
            // (k/x for query length k and keyword length x; 1 for complete keyword)
            match.setRelevance((float) enteredKey.length() / (float) key.length());
            
            matches.append(match);
        }
    }
    
    // Feed the framework with the calculated results
    context.addMatches(matches);
}

void Symbols::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context);
    QApplication::clipboard()->setText(match.text());
}

K_EXPORT_PLASMA_RUNNER(symbols, Symbols)

#include "symbols.moc"
