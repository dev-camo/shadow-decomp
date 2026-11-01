#ifndef BOOST_CONFIG_HPP
#define BOOST_CONFIG_HPP

//  (C) Copyright John Maddock 2001, 2002.
//  (C) Copyright Darin Adler 2001.
//  (C) Copyright Peter Dimov 2001.
//  (C) Copyright David Abrahams 2001 - 2002.
//  (C) Copyright Beman Dawes 2001 - 2003.
//  (C) Copyright Stefan Slapeta 2004.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Adapted from Boost 1.32 config.hpp and config/compiler/metrowerks.hpp.
// This subset supplies only the exception configuration used by the adapted
// headers. Other Boost configuration choices remain resolved in those headers.
// An explicit BOOST_NO_EXCEPTIONS overrides compiler exception support.
#ifndef BOOST_NO_EXCEPTIONS
#if defined(__MWERKS__)
#if !__option(exceptions)
#define BOOST_NO_EXCEPTIONS
#endif
#endif
#endif

#endif
