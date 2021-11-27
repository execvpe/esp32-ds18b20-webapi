#pragma once
#ifndef STRINGMACROS_HPP
#define STRINGMACROS_HPP

#define S_EQUALS(X, Y) (!strcmp(X, Y))
#define S_MATCH_FIRST(X, Y, N) (!strncmp(X, Y, N))

#endif	// STRINGMACROS_HPP