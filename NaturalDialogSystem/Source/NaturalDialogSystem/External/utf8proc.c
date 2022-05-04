/* -*- mode: c; c-basic-offset: 2; tab-width: 2; indent-tabs-mode: nil -*- */
/*
 *  Copyright (c) 2014-2021 Steven G. Johnson, Jiahao Chen, Peter Colberg, Tony Kelman, Scott P. Jones, and other contributors.
 *  Copyright (c) 2009 Public Software Group e. V., Berlin, Germany
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

/*
 *  This library contains derived data from a modified version of the
 *  Unicode data files.
 *
 *  The original data files are available at
 *  https://www.unicode.org/Public/UNIDATA/
 *
 *  Please notice the copyright statement in the file "utf8proc_data.c".
 */


/*
 *  File name:    utf8proc.c
 *
 *  Description:
 *  Implementation of libutf8proc.
 */


#include "utf8proc.h"

#ifndef SSIZE_MAX
#define SSIZE_MAX ((size_t)SIZE_MAX/2)
#endif
#ifndef UINT16_MAX
#  define UINT16_MAX 65535U
#endif

#include "utf8proc_data.c"


const utf8proc_int8_t utf8proc_utf8class[256] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0};

#define UTF8PROC_HANGUL_SBASE 0xAC00
#define UTF8PROC_HANGUL_LBASE 0x1100
#define UTF8PROC_HANGUL_VBASE 0x1161
#define UTF8PROC_HANGUL_TBASE 0x11A7
#define UTF8PROC_HANGUL_LCOUNT 19
#define UTF8PROC_HANGUL_VCOUNT 21
#define UTF8PROC_HANGUL_TCOUNT 28
#define UTF8PROC_HANGUL_NCOUNT 588
#define UTF8PROC_HANGUL_SCOUNT 11172
/* END is exclusive */
#define UTF8PROC_HANGUL_L_START  0x1100
#define UTF8PROC_HANGUL_L_END    0x115A
#define UTF8PROC_HANGUL_L_FILLER 0x115F
#define UTF8PROC_HANGUL_V_START  0x1160
#define UTF8PROC_HANGUL_V_END    0x11A3
#define UTF8PROC_HANGUL_T_START  0x11A8
#define UTF8PROC_HANGUL_T_END    0x11FA
#define UTF8PROC_HANGUL_S_START  0xAC00
#define UTF8PROC_HANGUL_S_END    0xD7A4

/* Should follow semantic-versioning rules (semver.org) based on API
   compatibility.  (Note that the shared-library version number will
   be different, being based on ABI compatibility.): */
#define STRINGIZEx(x) #x
#define STRINGIZE(x) STRINGIZEx(x)

const char* utf8proc_version(void)
{
	return STRINGIZE(UTF8PROC_VERSION_MAJOR) "." STRINGIZE(UTF8PROC_VERSION_MINOR) "." STRINGIZE(UTF8PROC_VERSION_PATCH) "";
}

const char* utf8proc_unicode_version(void)
{
	return "14.0.0";
}

const char* utf8proc_errmsg(utf8proc_ssize_t errcode)
{
	switch (errcode)
	{
		case UTF8PROC_ERROR_NOMEM:
			return "Memory for processing UTF-8 data could not be allocated.";
		case UTF8PROC_ERROR_OVERFLOW:
			return "UTF-8 string is too long to be processed.";
		case UTF8PROC_ERROR_INVALIDUTF8:
			return "Invalid UTF-8 string";
		case UTF8PROC_ERROR_NOTASSIGNED:
			return "Unassigned Unicode code point found in UTF-8 string.";
		case UTF8PROC_ERROR_INVALIDOPTS:
			return "Invalid options for UTF-8 processing chosen.";
		default:
			return "An unknown error occurred while processing UTF-8 data.";
	}
}

#define utf_cont(ch)  (((ch) & 0xc0) == 0x80)

utf8proc_ssize_t utf8proc_iterate(
	const utf8proc_uint8_t* str, utf8proc_ssize_t strlen, utf8proc_int32_t* dst
	)
{
	utf8proc_int32_t uc;
	const utf8proc_uint8_t* end;

	*dst = -1;
	if (!strlen) return 0;
	end = str + ((strlen < 0) ? 4 : strlen);
	uc = *str++;
	if (uc < 0x80)
	{
		*dst = uc;
		return 1;
	}
	// Must be between 0xc2 and 0xf4 inclusive to be valid
	if ((utf8proc_uint32_t)(uc - 0xc2) > (0xf4 - 0xc2)) return UTF8PROC_ERROR_INVALIDUTF8;
	if (uc < 0xe0)
	{
		// 2-byte sequence
		// Must have valid continuation character
		if (str >= end || !utf_cont(*str)) return UTF8PROC_ERROR_INVALIDUTF8;
		*dst = ((uc & 0x1f) << 6) | (*str & 0x3f);
		return 2;
	}
	if (uc < 0xf0)
	{
		// 3-byte sequence
		if ((str + 1 >= end) || !utf_cont(*str) || !utf_cont(str[1]))
			return UTF8PROC_ERROR_INVALIDUTF8;
		// Check for surrogate chars
		if (uc == 0xed && *str > 0x9f)
			return UTF8PROC_ERROR_INVALIDUTF8;
		uc = ((uc & 0xf) << 12) | ((*str & 0x3f) << 6) | (str[1] & 0x3f);
		if (uc < 0x800)
			return UTF8PROC_ERROR_INVALIDUTF8;
		*dst = uc;
		return 3;
	}
	// 4-byte sequence
	// Must have 3 valid continuation characters
	if ((str + 2 >= end) || !utf_cont(*str) || !utf_cont(str[1]) || !utf_cont(str[2]))
		return UTF8PROC_ERROR_INVALIDUTF8;
	// Make sure in correct range (0x10000 - 0x10ffff)
	if (uc == 0xf0)
	{
		if (*str < 0x90) return UTF8PROC_ERROR_INVALIDUTF8;
	}
	else if (uc == 0xf4)
	{
		if (*str > 0x8f) return UTF8PROC_ERROR_INVALIDUTF8;
	}
	*dst = ((uc & 7) << 18) | ((*str & 0x3f) << 12) | ((str[1] & 0x3f) << 6) | (str[2] & 0x3f);
	return 4;
}

utf8proc_bool utf8proc_codepoint_valid(utf8proc_int32_t uc)
{
	return (((utf8proc_uint32_t)uc) - 0xd800 > 0x07ff) && ((utf8proc_uint32_t)uc < 0x110000);
}

utf8proc_ssize_t utf8proc_encode_char(utf8proc_int32_t uc, utf8proc_uint8_t* dst)
{
	if (uc < 0x00)
	{
		return 0;
	}
	else if (uc < 0x80)
	{
		dst[0] = (utf8proc_uint8_t)uc;
		return 1;
	}
	else if (uc < 0x800)
	{
		dst[0] = (utf8proc_uint8_t)(0xC0 + (uc >> 6));
		dst[1] = (utf8proc_uint8_t)(0x80 + (uc & 0x3F));
		return 2;
		// Note: we allow encoding 0xd800-0xdfff here, so as not to change
		// the API, however, these are actually invalid in UTF-8
	}
	else if (uc < 0x10000)
	{
		dst[0] = (utf8proc_uint8_t)(0xE0 + (uc >> 12));
		dst[1] = (utf8proc_uint8_t)(0x80 + ((uc >> 6) & 0x3F));
		dst[2] = (utf8proc_uint8_t)(0x80 + (uc & 0x3F));
		return 3;
	}
	else if (uc < 0x110000)
	{
		dst[0] = (utf8proc_uint8_t)(0xF0 + (uc >> 18));
		dst[1] = (utf8proc_uint8_t)(0x80 + ((uc >> 12) & 0x3F));
		dst[2] = (utf8proc_uint8_t)(0x80 + ((uc >> 6) & 0x3F));
		dst[3] = (utf8proc_uint8_t)(0x80 + (uc & 0x3F));
		return 4;
	}
	else return 0;
}

/* internal version used for inserting 0xff bytes between graphemes */
static utf8proc_ssize_t charbound_encode_char(utf8proc_int32_t uc, utf8proc_uint8_t* dst)
{
	if (uc < 0x00)
	{
		if (uc == -1)
		{
			/* internal value used for grapheme breaks */
			dst[0] = (utf8proc_uint8_t)0xFF;
			return 1;
		}
		return 0;
	}
	else if (uc < 0x80)
	{
		dst[0] = (utf8proc_uint8_t)uc;
		return 1;
	}
	else if (uc < 0x800)
	{
		dst[0] = (utf8proc_uint8_t)(0xC0 + (uc >> 6));
		dst[1] = (utf8proc_uint8_t)(0x80 + (uc & 0x3F));
		return 2;
	}
	else if (uc < 0x10000)
	{
		dst[0] = (utf8proc_uint8_t)(0xE0 + (uc >> 12));
		dst[1] = (utf8proc_uint8_t)(0x80 + ((uc >> 6) & 0x3F));
		dst[2] = (utf8proc_uint8_t)(0x80 + (uc & 0x3F));
		return 3;
	}
	else if (uc < 0x110000)
	{
		dst[0] = (utf8proc_uint8_t)(0xF0 + (uc >> 18));
		dst[1] = (utf8proc_uint8_t)(0x80 + ((uc >> 12) & 0x3F));
		dst[2] = (utf8proc_uint8_t)(0x80 + ((uc >> 6) & 0x3F));
		dst[3] = (utf8proc_uint8_t)(0x80 + (uc & 0x3F));
		return 4;
	}
	else return 0;
}

/* internal "unsafe" version that does not check whether uc is in range */
static const utf8proc_property_t* unsafe_get_property(utf8proc_int32_t uc)
{
	/* ASSERT: uc >= 0 && uc < 0x110000 */
	return utf8proc_properties + (
		       utf8proc_stage2table[
			       utf8proc_stage1table[uc >> 8] + (uc & 0xFF)
		       ]
	       );
}

const utf8proc_property_t* utf8proc_get_property(utf8proc_int32_t uc)
{
	return uc < 0 || uc >= 0x110000 ? utf8proc_properties : unsafe_get_property(uc);
}

/* return whether there is a grapheme break between boundclasses lbc and tbc
   (according to the definition of extended grapheme clusters)

  Rule numbering refers to TR29 Version 29 (Unicode 9.0.0):
  http://www.unicode.org/reports/tr29/tr29-29.html

  CAVEATS:
   Please note that evaluation of GB10 (grapheme breaks between emoji zwj sequences)
   and GB 12/13 (regional indicator code points) require knowledge of previous characters
   and are thus not handled by this function. This may result in an incorrect break before
   an E_Modifier class codepoint and an incorrectly missing break between two
   REGIONAL_INDICATOR class code points if such support does not exist in the caller.

   See the special support in grapheme_break_extended, for required bookkeeping by the caller.
*/
static utf8proc_bool grapheme_break_simple(int lbc, int tbc)
{
	return
		(lbc == UTF8PROC_BOUNDCLASS_START)
			? true
			:                                 // GB1
			(lbc == UTF8PROC_BOUNDCLASS_CR && // GB3
			 tbc == UTF8PROC_BOUNDCLASS_LF)
			? false
			: // ---
			(lbc >= UTF8PROC_BOUNDCLASS_CR && lbc <= UTF8PROC_BOUNDCLASS_CONTROL)
			? true
			: // GB4
			(tbc >= UTF8PROC_BOUNDCLASS_CR && tbc <= UTF8PROC_BOUNDCLASS_CONTROL)
			? true
			:                                  // GB5
			(lbc == UTF8PROC_BOUNDCLASS_L &&   // GB6
			 (tbc == UTF8PROC_BOUNDCLASS_L ||  // ---
			  tbc == UTF8PROC_BOUNDCLASS_V ||  // ---
			  tbc == UTF8PROC_BOUNDCLASS_LV || // ---
			  tbc == UTF8PROC_BOUNDCLASS_LVT))
			? false
			:                                  // ---
			((lbc == UTF8PROC_BOUNDCLASS_LV || // GB7
			  lbc == UTF8PROC_BOUNDCLASS_V) && // ---
			 (tbc == UTF8PROC_BOUNDCLASS_V ||  // ---
			  tbc == UTF8PROC_BOUNDCLASS_T))
			? false
			:                                   // ---
			((lbc == UTF8PROC_BOUNDCLASS_LVT || // GB8
			  lbc == UTF8PROC_BOUNDCLASS_T) &&  // ---
			 tbc == UTF8PROC_BOUNDCLASS_T)
			? false
			:                                          // ---
			(tbc == UTF8PROC_BOUNDCLASS_EXTEND ||      // GB9
			 tbc == UTF8PROC_BOUNDCLASS_ZWJ ||         // ---
			 tbc == UTF8PROC_BOUNDCLASS_SPACINGMARK || // GB9a
			 lbc == UTF8PROC_BOUNDCLASS_PREPEND)
			? false
			:                                    // GB9b
			(lbc == UTF8PROC_BOUNDCLASS_E_ZWG && // GB11 (requires additional handling below)
			 tbc == UTF8PROC_BOUNDCLASS_EXTENDED_PICTOGRAPHIC)
			? false
			:                                                 // ----
			(lbc == UTF8PROC_BOUNDCLASS_REGIONAL_INDICATOR && // GB12/13 (requires additional handling below)
			 tbc == UTF8PROC_BOUNDCLASS_REGIONAL_INDICATOR)
			? false
			:     // ----
			true; // GB999
}

static utf8proc_bool grapheme_break_extended(int lbc, int tbc, utf8proc_int32_t* state)
{
	if (state)
	{
		int lbc_override;
		if (*state == UTF8PROC_BOUNDCLASS_START)
			*state = lbc_override = lbc;
		else
			lbc_override = *state;
		utf8proc_bool break_permitted = grapheme_break_simple(lbc_override, tbc);

		// Special support for GB 12/13 made possible by GB999. After two RI
		// class codepoints we want to force a break. Do this by resetting the
		// second RI's bound class to UTF8PROC_BOUNDCLASS_OTHER, to force a break
		// after that character according to GB999 (unless of course such a break is
		// forbidden by a different rule such as GB9).
		if (*state == tbc && tbc == UTF8PROC_BOUNDCLASS_REGIONAL_INDICATOR)
			*state = UTF8PROC_BOUNDCLASS_OTHER;
			// Special support for GB11 (emoji extend* zwj / emoji)
		else if (*state == UTF8PROC_BOUNDCLASS_EXTENDED_PICTOGRAPHIC)
		{
			if (tbc == UTF8PROC_BOUNDCLASS_EXTEND) // fold EXTEND codepoints into emoji
				*state = UTF8PROC_BOUNDCLASS_EXTENDED_PICTOGRAPHIC;
			else if (tbc == UTF8PROC_BOUNDCLASS_ZWJ)
				*state = UTF8PROC_BOUNDCLASS_E_ZWG; // state to record emoji+zwg combo
			else
				*state = tbc;
		}
		else
			*state = tbc;

		return break_permitted;
	}
	else
		return grapheme_break_simple(lbc, tbc);
}

utf8proc_bool utf8proc_grapheme_break_stateful(
	utf8proc_int32_t c1, utf8proc_int32_t c2, utf8proc_int32_t* state)
{
	return grapheme_break_extended(utf8proc_get_property(c1)->boundclass,
		utf8proc_get_property(c2)->boundclass,
		state);
}


utf8proc_bool utf8proc_grapheme_break(
	utf8proc_int32_t c1, utf8proc_int32_t c2)
{
	return utf8proc_grapheme_break_stateful(c1, c2, NULL);
}

static utf8proc_int32_t seqindex_decode_entry(const utf8proc_uint16_t** entry)
{
	utf8proc_int32_t entry_cp = **entry;
	if ((entry_cp & 0xF800) == 0xD800)
	{
		*entry = *entry + 1;
		entry_cp = ((entry_cp & 0x03FF) << 10) | (**entry & 0x03FF);
		entry_cp += 0x10000;
	}
	return entry_cp;
}

static utf8proc_int32_t seqindex_decode_index(const utf8proc_uint32_t seqindex)
{
	const utf8proc_uint16_t* entry = &utf8proc_sequences[seqindex];
	return seqindex_decode_entry(&entry);
}

utf8proc_int32_t utf8proc_tolower(utf8proc_int32_t c)
{
	utf8proc_int32_t cl = utf8proc_get_property(c)->lowercase_seqindex;
	return cl != UINT16_MAX ? seqindex_decode_index((utf8proc_uint32_t)cl) : c;
}

utf8proc_int32_t utf8proc_toupper(utf8proc_int32_t c)
{
	utf8proc_int32_t cu = utf8proc_get_property(c)->uppercase_seqindex;
	return cu != UINT16_MAX ? seqindex_decode_index((utf8proc_uint32_t)cu) : c;
}

utf8proc_int32_t utf8proc_totitle(utf8proc_int32_t c)
{
	utf8proc_int32_t cu = utf8proc_get_property(c)->titlecase_seqindex;
	return cu != UINT16_MAX ? seqindex_decode_index((utf8proc_uint32_t)cu) : c;
}

int utf8proc_islower(utf8proc_int32_t c)
{
	const utf8proc_property_t* p = utf8proc_get_property(c);
	return p->lowercase_seqindex != p->uppercase_seqindex && p->lowercase_seqindex == UINT16_MAX;
}

int utf8proc_isupper(utf8proc_int32_t c)
{
	const utf8proc_property_t* p = utf8proc_get_property(c);
	return p->lowercase_seqindex != p->uppercase_seqindex && p->uppercase_seqindex == UINT16_MAX && p->category != UTF8PROC_CATEGORY_LT;
}

/* return a character width analogous to wcwidth (except portable and
   hopefully less buggy than most system wcwidth functions). */
int utf8proc_charwidth(utf8proc_int32_t c)
{
	return utf8proc_get_property(c)->charwidth;
}

utf8proc_category_t utf8proc_category(utf8proc_int32_t c)
{
	return (utf8proc_category_t)utf8proc_get_property(c)->category;
}

const char* utf8proc_category_string(utf8proc_int32_t c)
{
	static const char s[][3] = {"Cn", "Lu", "Ll", "Lt", "Lm", "Lo", "Mn", "Mc", "Me", "Nd", "Nl", "No", "Pc", "Pd", "Ps", "Pe", "Pi", "Pf", "Po", "Sm", "Sc", "Sk", "So", "Zs", "Zl", "Zp", "Cc", "Cf", "Cs", "Co"};
	return s[utf8proc_category(c)];
}

#define utf8proc_decompose_lump(replacement_uc) \
  return utf8proc_decompose_char((replacement_uc), dst, bufsize, \
  options & ~(unsigned int)UTF8PROC_LUMP, last_boundclass)


utf8proc_ssize_t utf8proc_normalize_utf32(utf8proc_int32_t* buffer, utf8proc_ssize_t length, utf8proc_option_t options)
{
	/* UTF8PROC_NULLTERM option will be ignored, 'length' is never ignored */
	if (options & (UTF8PROC_NLF2LS | UTF8PROC_NLF2PS | UTF8PROC_STRIPCC))
	{
		utf8proc_ssize_t rpos;
		utf8proc_ssize_t wpos = 0;
		utf8proc_int32_t uc;
		for (rpos = 0; rpos < length; rpos++)
		{
			uc = buffer[rpos];
			if (uc == 0x000D && rpos < length - 1 && buffer[rpos + 1] == 0x000A) rpos++;
			if (uc == 0x000A || uc == 0x000D || uc == 0x0085 ||
			    ((options & UTF8PROC_STRIPCC) && (uc == 0x000B || uc == 0x000C)))
			{
				if (options & UTF8PROC_NLF2LS)
				{
					if (options & UTF8PROC_NLF2PS)
					{
						buffer[wpos++] = 0x000A;
					}
					else
					{
						buffer[wpos++] = 0x2028;
					}
				}
				else
				{
					if (options & UTF8PROC_NLF2PS)
					{
						buffer[wpos++] = 0x2029;
					}
					else
					{
						buffer[wpos++] = 0x0020;
					}
				}
			}
			else if ((options & UTF8PROC_STRIPCC) &&
			         (uc < 0x0020 || (uc >= 0x007F && uc < 0x00A0)))
			{
				if (uc == 0x0009) buffer[wpos++] = 0x0020;
			}
			else
			{
				buffer[wpos++] = uc;
			}
		}
		length = wpos;
	}
	if (options & UTF8PROC_COMPOSE)
	{
		utf8proc_int32_t* starter = NULL;
		utf8proc_int32_t current_char;
		const utf8proc_property_t* starter_property = NULL,* current_property;
		utf8proc_propval_t max_combining_class = -1;
		utf8proc_ssize_t rpos;
		utf8proc_ssize_t wpos = 0;
		utf8proc_int32_t composition;
		for (rpos = 0; rpos < length; rpos++)
		{
			current_char = buffer[rpos];
			current_property = unsafe_get_property(current_char);
			if (starter && current_property->combining_class > max_combining_class)
			{
				/* combination perhaps possible */
				utf8proc_int32_t hangul_lindex;
				utf8proc_int32_t hangul_sindex;
				hangul_lindex = *starter - UTF8PROC_HANGUL_LBASE;
				if (hangul_lindex >= 0 && hangul_lindex < UTF8PROC_HANGUL_LCOUNT)
				{
					utf8proc_int32_t hangul_vindex;
					hangul_vindex = current_char - UTF8PROC_HANGUL_VBASE;
					if (hangul_vindex >= 0 && hangul_vindex < UTF8PROC_HANGUL_VCOUNT)
					{
						*starter = UTF8PROC_HANGUL_SBASE +
						           (hangul_lindex * UTF8PROC_HANGUL_VCOUNT + hangul_vindex) *
						           UTF8PROC_HANGUL_TCOUNT;
						starter_property = NULL;
						continue;
					}
				}
				hangul_sindex = *starter - UTF8PROC_HANGUL_SBASE;
				if (hangul_sindex >= 0 && hangul_sindex < UTF8PROC_HANGUL_SCOUNT &&
				    (hangul_sindex % UTF8PROC_HANGUL_TCOUNT) == 0)
				{
					utf8proc_int32_t hangul_tindex;
					hangul_tindex = current_char - UTF8PROC_HANGUL_TBASE;
					if (hangul_tindex >= 0 && hangul_tindex < UTF8PROC_HANGUL_TCOUNT)
					{
						*starter += hangul_tindex;
						starter_property = NULL;
						continue;
					}
				}
				if (!starter_property)
				{
					starter_property = unsafe_get_property(*starter);
				}
				if (starter_property->comb_index < 0x8000 &&
				    current_property->comb_index != UINT16_MAX &&
				    current_property->comb_index >= 0x8000)
				{
					int sidx = starter_property->comb_index;
					int idx = current_property->comb_index & 0x3FFF;
					if (idx >= utf8proc_combinations[sidx] && idx <= utf8proc_combinations[sidx + 1])
					{
						idx += sidx + 2 - utf8proc_combinations[sidx];
						if (current_property->comb_index & 0x4000)
						{
							composition = (utf8proc_combinations[idx] << 16) | utf8proc_combinations[idx + 1];
						}
						else
							composition = utf8proc_combinations[idx];

						if (composition > 0 && (!(options & UTF8PROC_STABLE) ||
						                        !(unsafe_get_property(composition)->comp_exclusion)))
						{
							*starter = composition;
							starter_property = NULL;
							continue;
						}
					}
				}
			}
			buffer[wpos] = current_char;
			if (current_property->combining_class)
			{
				if (current_property->combining_class > max_combining_class)
				{
					max_combining_class = current_property->combining_class;
				}
			}
			else
			{
				starter = buffer + wpos;
				starter_property = NULL;
				max_combining_class = -1;
			}
			wpos++;
		}
		length = wpos;
	}
	return length;
}

utf8proc_ssize_t utf8proc_reencode(utf8proc_int32_t* buffer, utf8proc_ssize_t length, utf8proc_option_t options)
{
	/* UTF8PROC_NULLTERM option will be ignored, 'length' is never ignored
	   ASSERT: 'buffer' has one spare byte of free space at the end! */
	length = utf8proc_normalize_utf32(buffer, length, options);
	if (length < 0) return length;
	{
		utf8proc_ssize_t rpos, wpos = 0;
		utf8proc_int32_t uc;
		if (options & UTF8PROC_CHARBOUND)
		{
			for (rpos = 0; rpos < length; rpos++)
			{
				uc = buffer[rpos];
				wpos += charbound_encode_char(uc, ((utf8proc_uint8_t*)buffer) + wpos);
			}
		}
		else
		{
			for (rpos = 0; rpos < length; rpos++)
			{
				uc = buffer[rpos];
				wpos += utf8proc_encode_char(uc, ((utf8proc_uint8_t*)buffer) + wpos);
			}
		}
		((utf8proc_uint8_t*)buffer)[wpos] = 0;
		return wpos;
	}
}