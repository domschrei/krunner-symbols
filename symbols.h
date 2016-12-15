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

#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <KRunner/AbstractRunner>

class Symbols : public Plasma::AbstractRunner
{
    Q_OBJECT

public:
    Symbols(QObject *parent, const QVariantList &args);
    ~Symbols();

    void match(Plasma::RunnerContext &);
    void matchUnicode(Plasma::RunnerContext &);
    void run(const Plasma::RunnerContext &, const Plasma::QueryMatch &);
    
private:
    QMap<QString, QString> symbols;
    QMap<QStringList, QString> unicodeSymbols;
};

#endif
