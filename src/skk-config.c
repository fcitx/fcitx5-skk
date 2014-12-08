/***************************************************************************
 *   Copyright (C) 2013~2013 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *  This program is free software: you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "skk.h"

CONFIG_BINDING_BEGIN(FcitxSkkConfig)
CONFIG_BINDING_REGISTER("General", "PunctuationStyle", punctuationStyle)
CONFIG_BINDING_REGISTER("General", "InitialInputMode", initialInputMode)
CONFIG_BINDING_REGISTER("General", "PageSize", pageSize)
CONFIG_BINDING_REGISTER("General", "CandidateLayout", candidateLayout)
CONFIG_BINDING_REGISTER("General", "NTriggersToShowCandWin", nTriggersToShowCandWin)
CONFIG_BINDING_REGISTER("General", "ShowAnnotation", showAnnotation)
CONFIG_BINDING_REGISTER("General", "EggLikeNewLine", eggLikeNewLine)
CONFIG_BINDING_REGISTER("General", "CandidateChooseKey", candidateChooseKey) /* candidate selection keys */
CONFIG_BINDING_END()
