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
        
            if (foundKey.length() == enteredKey.length())
	    {
                // the query equals the keyword -> exact match
                match.setType(Plasma::QueryMatch::ExactMatch);
		match.setText(it.value());
	    } else {
                // the query is a (non-complete) prefix of the keyword -> completion match
                match.setType(Plasma::QueryMatch::CompletionMatch);
		// also show the exact keyword for this value
		match.setText(it.value() + "  [" + foundKey + "] ");
	    }
	    
	    // Check if the result is a command ("open:" or "exec:")
	    if (it.value().startsWith("open:"))
	    {
		match.setText(match.text().replace("open:", "→ "));
	    } else if (it.value().startsWith("exec:")) {
		match.setText(match.text().replace("exec:", "→ "));
	    }
            
            // Basic properties for the match
            match.setIcon(QIcon::fromTheme("preferences-desktop-font"));
	    match.setSubtext(it.value());
	    
            // The match's relevance gets higher the more "complete" the query string is
            // (k/x for query length k and keyword length x; 1 for complete keyword)
            match.setRelevance((float) enteredKey.length() / (float) foundKey.length());
            
            matches.append(match);
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

    if (match.subtext().startsWith("open:"))
    {
	// Open a file or a URL in a (file/web) browser
	// (in a new process, so that krunner doesn't get stuck while opening the path)
	string command = "kde-open " + match.subtext().remove("open:").toStdString() + " &";
        system(command.c_str());
	
    } else if (match.subtext().startsWith("exec:")) 
    {
	// Execute a command
	// (in a new process, so that krunner doesn't get stuck while opening the path)
	string command = match.subtext().remove("exec:").toStdString() + " &";
        system(command.c_str());
	
    } else 
    {
	// Copy the result to clipboard
	QApplication::clipboard()->setText(match.subtext());
    }
}

K_EXPORT_PLASMA_RUNNER(symbols, Symbols)

#include "symbols.moc"
