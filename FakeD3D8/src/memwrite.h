/*
 * Copyright � 2014, 2015, 2016 L�szl� J�zsef Cs�ndes
 *
 * This file is part of FLSubs.
 *
 * FLSubs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FLSubs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FLSubs.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "global.h"

/**
 * Does *target=data, but unlocks the page(s) first
 * @throw std::runtime_error on failure.
 */
void WriteProtectedPtr(void** target, void* data);

/**
 * Overwrites an entry in an object's vtable (with protection)
 * @return The old function.
 */
void* SwapVPtr(void* object, uint ordinal, void* function);
