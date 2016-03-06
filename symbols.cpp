/******************************************************************************
 *  Copyright (C) 2016 by Dominik Schreiber <mail@dominikschreiber.de>        *
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
 *  along with this library; see the file COPYING.LIB.                        *
 *  If not, see <http://www.gnu.org/licenses/>.                               *
 *****************************************************************************/

#include "symbols.h"

#include <KLocalizedString>
#include <QApplication>
#include <QClipboard>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
using namespace std;

Symbols::Symbols(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);
    
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
            i18n("Sucht nach einem Unicode-Symbol der Beschreibung :q: und zeigt es, falls vorhanden, an.")
        )
    );
    
    string line;
    ifstream file ("/home/dominik/workspace/krunner-symbols/symbols.conf");
    if (file.is_open())
    {
        while ( getline (file,line) )
        {
            int index = line.find("=", 0);
            symbols[line.substr(0, index)] = line.substr(index + 1);
        }
        file.close();
    }
    else cout << "Unable to open file";
}

Symbols::~Symbols()
{
}

void Symbols::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();    
    if (!context.isValid()) return;
    
    if (symbols.count(term.toStdString()) == 1)
    {
        QList<Plasma::QueryMatch> matches;
        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::InformationalMatch);
        match.setIcon(QIcon::fromTheme("applications-education-language"));
        string symbol = symbols[term.toStdString()];
        match.setText(QString::fromStdString(symbol));
        match.setRelevance(1);
        matches.append(match);
        
        context.addMatches(matches);
    }
}

void Symbols::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context);
    QApplication::clipboard()->setText(match.text());
}

K_EXPORT_PLASMA_RUNNER(symbols, Symbols)

#include "symbols.moc"
