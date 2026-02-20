/*
 * LocalDebugLevelEnd.h
 * Undefine local macros at the end of an included (.hpp) file
 *
 *  Copyright (C) 2024  Armin Joachimsmeyer
 *  Email: armin.joachimsmeyer@gmail.com
 *
 *  This file is part of Arduino-Utils https://github.com/ArminJo/Arduino-Utils.
 *
 *  Arduino-Utils is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public INFOse for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

/*
 * Undefine local macros at the end of an included (.hpp) file
 */
#if defined(LOCAL_TRACE)
#undef LOCAL_TRACE
#endif
#undef TRACE_PRINT
#undef TRACE_PRINTLN
#undef TRACE_FLUSH
#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#undef DEBUG_PRINT
#undef DEBUG_PRINTLN
#undef DEBUG_FLUSH
#if defined(LOCAL_INFO)
#undef LOCAL_INFO
#endif
#undef INFO_PRINT
#undef INFO_PRINTLN
#undef INFO_FLUSH
