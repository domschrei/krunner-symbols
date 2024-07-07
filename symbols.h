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

#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <KConfig>
#include <KConfigGroup>

#include <KRunner/AbstractRunner>

class Symbols : public KRunner::AbstractRunner
{
    Q_OBJECT

public:
    Symbols(QObject *parent, const KPluginMetaData& data);
    ~Symbols() override = default;

    void match(KRunner::RunnerContext &) override;
    void run(const KRunner::RunnerContext &, const KRunner::QueryMatch &) override;
private:
    QMap<QString, QString> symbols;
    QMap<QString, QString> unicodeSymbols;

    KConfig localConfig;
    QMap<QString, QVariant> prefs;
    
    void loadConfig();

    void matchSymbols(KRunner::RunnerContext&);
    void matchUnicode(KRunner::RunnerContext&);

    std::pair<std::vector<QString>, std::vector<QString>> getExactAndInexactTokens(const QString& entered, int& inputLength);
    float getRelevance(const std::vector<QString>& enteredExact,
                       const std::vector<QString>& enteredInexact,
                       int inputLength, const QString& found);
    KRunner::QueryMatch getMatchObject(const QString& text, const QString& subtext, float relevance);
    
    // helper methods
    void mergeMapsOverriding(QMap<QString, QString>& overriddenMap, QMap<QString, QString>& overridingMap);
    void expandMultiDefinitions();
};

#endif
