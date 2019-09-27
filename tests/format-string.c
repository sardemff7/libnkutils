/*
 * libnkutils/format-string - Miscellaneous utilities, format string module
 *
 * Copyright © 2011-2017 Quentin "Sardem FF7" Glidic
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <locale.h>

#include <glib.h>

#include "nkutils-format-string.h"

#define MAX_DATA 4

typedef struct {
    const gchar *name;
    const gchar *content;
} NkFormatStringTestDataData;

typedef struct {
    gunichar identifier;
    const gchar *source;
    NkFormatStringTestDataData data[MAX_DATA + 1];
    gint error;
    const gchar *result;
} NkFormatStringTestData;

static const struct {
    const gchar *testpath;
    NkFormatStringTestData data;
} _nk_format_string_tests_list[] = {
    {
        .testpath = "/nkutils/name/basic",
        .data = {
            .identifier = '$',
            .source = "You can make ${recipe} with ${fruit}.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'a banana split'" },
                { .name = NULL }
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/basic/dash-underscore",
        .data = {
            .identifier = '$',
            .source = "You can make ${recipe_name} with ${fruit-name}.",
            .data = {
                { .name = "fruit-name", .content = "'a banana'" },
                { .name = "recipe_name", .content = "'a banana split'" },
                { .name = NULL }
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/basic/unicode",
        .data = {
            .identifier = '$',
            .source = "You can make ${recette} with ${ingrédient}.",
            .data = {
                { .name = "ingrédient", .content = "'a banana'" },
                { .name = "recette", .content = "'a banana split'" },
                { .name = NULL }
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/basic/boolean/true",
        .data = {
            .identifier = '$',
            .source = "You are ${bool}.",
            .data = {
                { .name = "bool", .content = "true" },
                { .name = NULL }
            },
            .result = "You are true."
        }
    },
    {
        .testpath = "/nkutils/name/basic/boolean/false",
        .data = {
            .identifier = '$',
            .source = "You are ${bool}.",
            .data = {
                { .name = "bool", .content = "false" },
                { .name = NULL }
            },
            .result = "You are false."
        }
    },
    {
        .testpath = "/nkutils/name/basic/wrong/1",
        .data = {
            .identifier = '$',
            .source = "You can make ${recipe} with $fruit.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'a banana split'" },
                { .name = NULL }
            },
            .result = "You can make a banana split with $fruit."
        }
    },
    {
        .testpath = "/nkutils/name/basic/wrong/2",
        .data = {
            .identifier = '$',
            .source = "$fruit is good.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'a banana split'" },
                { .name = NULL }
            },
            .result = "$fruit is good."
        }
    },
    {
        .testpath = "/nkutils/name/key/index/positive",
        .data = {
            .identifier = '$',
            .source = "You can make a ${recipe[0]} with ${fruit}.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "['banana split', 'apple pie']" },
                { .name = NULL }
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/key/index/negative",
        .data = {
            .identifier = '$',
            .source = "You can make a ${recipe[-1]} with ${fruit}.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "['apple pie', 'banana split']" },
                { .name = NULL }
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/key/name",
        .data = {
            .identifier = '$',
            .source = "You can make a ${recipe[icecream]} with ${fruit}.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "{'icecream': 'banana split'}" },
                { .name = NULL }
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/key/name/modifier",
        .data = {
            .identifier = '$',
            .source = "You can make a ${recipe[cake]:-banana cake} with ${fruit}.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "{'cream': 'banana split'}" },
                { .name = NULL }
            },
            .result = "You can make a banana cake with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/key/join/default",
        .data = {
            .identifier = '$',
            .source = "You can make [${recipes[@]}] with ${fruit}.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipes", .content = "['banana pie', 'banana split']" },
                { .name = NULL }
            },
            .result = "You can make [banana pie, banana split] with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/key/join/custom",
        .data = {
            .identifier = '$',
            .source = "You can make [${recipes[@; ]}] with ${fruit}.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipes", .content = "['banana pie', 'banana split']" },
                { .name = NULL }
            },
            .result = "You can make [banana pie; banana split] with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/key/join/replace",
        .data = {
            .identifier = '$',
            .source = "You can make [${recipes[@@]/@/], [}] with ${fruit}.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipes", .content = "['banana pie', 'banana split']" },
                { .name = NULL }
            },
            .result = "You can make [banana pie], [banana split] with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/wrong/modifier",
        .data = {
            .identifier = '$',
            .source = "You can make a ${recipe} with ${fruit::}.",
            .error = NK_FORMAT_STRING_ERROR_UNKNOWN_MODIFIER,
        }
    },
    {
        .testpath = "/nkutils/name/wrong/key/index",
        .data = {
            .identifier = '$',
            .source = "You can make a ${recipe[18446744073709551616]} with ${fruit}.",
            .error = NK_FORMAT_STRING_ERROR_WRONG_KEY,
        }
    },
    {
        .testpath = "/nkutils/name/wrong/key",
        .data = {
            .identifier = '$',
            .source = "You can make a ${recipe[|]} with ${fruit}.",
            .error = NK_FORMAT_STRING_ERROR_WRONG_KEY,
        }
    },
    {
        .testpath = "/nkutils/name/wrong/regex/pattern",
        .data = {
            .identifier = '$',
            .source = "You can make a ${recipe/[} with ${fruit}.",
            .error = NK_FORMAT_STRING_ERROR_REGEX,
        }
    },
    {
        .testpath = "/nkutils/name/wrong/regex/replace",
        .data = {
            .identifier = '$',
            .source = "You can make ${recipe/a/\\gwrong} with ${fruit}.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'a banana split'" },
                { .name = NULL }
            },
            .result = "You can make  with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/fallback/with",
        .data = {
            .identifier = '$',
            .source = "I want to eat ${fruit:-an apple}.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = NULL }
            },
            .result = "I want to eat a banana."
        }
    },
    {
        .testpath = "/nkutils/name/fallback/without",
        .data = {
            .identifier = '$',
            .source = "I want to eat ${fruit:-an apple}.",
            .data = {
                { .name = NULL }
            },
            .result = "I want to eat an apple."
        }
    },
    {
        .testpath = "/nkutils/name/fallback/boolean/true",
        .data = {
            .identifier = '$',
            .source = "I want to eat a ${good:-bad} banana.",
            .data = {
                { .name = "good", .content = "true" },
                { .name = NULL }
            },
            .result = "I want to eat a true banana."
        }
    },
    {
        .testpath = "/nkutils/name/fallback/boolean/false",
        .data = {
            .identifier = '$',
            .source = "I want to eat a ${good:-bad} banana.",
            .data = {
                { .name = "good", .content = "false" },
                { .name = NULL }
            },
            .result = "I want to eat a bad banana."
        }
    },
    {
        .testpath = "/nkutils/name/fallback/recurse",
        .data = {
            .identifier = '$',
            .source = "I want to eat ${fruit:-${vegetable}}.",
            .data = {
                { .name = "vegetable", .content = "'a zucchini'" },
                { .name = NULL }
            },
            .result = "I want to eat a zucchini."
        }
    },
    {
        .testpath = "/nkutils/name/substitute/with",
        .data = {
            .identifier = '$',
            .source = "You can make a ${adjective:+(}${adjective}${adjective:+) }${recipe} with ${fruit}${addition:+ and }${addition}.",
            .data = {
                { .name = "adjective", .content = "'creamy'" },
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'banana split'" },
                { .name = "addition", .content = "'some cream'" },
                { .name = NULL }
            },
            .result = "You can make a (creamy) banana split with a banana and some cream."
        }
    },
    {
        .testpath = "/nkutils/name/substitute/without",
        .data = {
            .identifier = '$',
            .source = "You can make a ${adjective:+(}${adjective}${adjective:+) }${recipe} with ${fruit}${addition:+ and }${addition}.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'banana split'" },
                { .name = NULL }
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/substitute/boolean/true",
        .data = {
            .identifier = '$',
            .source = "You can make a ${good:+good }${recipe} with ${fruit}${addition:+ and }${addition}.",
            .data = {
                { .name = "good", .content = "true" },
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'banana split'" },
                { .name = NULL }
            },
            .result = "You can make a good banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/substitute/boolean/false",
        .data = {
            .identifier = '$',
            .source = "You can make a ${good:+good }${recipe} with ${fruit}${addition:+ and }${addition}.",
            .data = {
                { .name = "good", .content = "false" },
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'banana split'" },
                { .name = NULL }
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/anti-substitute/with",
        .data = {
            .identifier = '$',
            .source = "I want to eat a ${adjective:!sweat }lemon.",
            .data = {
                { .name = "adjective", .content = "'juicy'" },
                { .name = NULL }
            },
            .result = "I want to eat a lemon."
        }
    },
    {
        .testpath = "/nkutils/name/anti-substitute/without",
        .data = {
            .identifier = '$',
            .source = "I want to eat a ${adjective:!sweat }lemon.",
            .data = {
                { .name = NULL }
            },
            .result = "I want to eat a sweat lemon."
        }
    },
    {
        .testpath = "/nkutils/name/anti-substitute/boolean/true",
        .data = {
            .identifier = '$',
            .source = "I want to eat a ${good:!bad }lemon.",
            .data = {
                { .name = "good", .content = "true" },
                { .name = NULL }
            },
            .result = "I want to eat a lemon."
        }
    },
    {
        .testpath = "/nkutils/name/anti-substitute/boolean/false",
        .data = {
            .identifier = '$',
            .source = "I want to eat a ${good:!bad }lemon.",
            .data = {
                { .name = "good", .content = "false" },
                { .name = NULL }
            },
            .result = "I want to eat a bad lemon."
        }
    },
    {
        .testpath = "/nkutils/name/switch/true",
        .data = {
            .identifier = '$',
            .source = "Active: ${active:{;yes;no}}.",
            .data = {
                { .name = "active", .content = "true" },
                { .name = NULL }
            },
            .result = "Active: yes."
        }
    },
    {
        .testpath = "/nkutils/name/switch/false",
        .data = {
            .identifier = '$',
            .source = "Active: ${active:{;yes;no}}.",
            .data = {
                { .name = "active", .content = "false" },
                { .name = NULL }
            },
            .result = "Active: no."
        }
    },
    {
        .testpath = "/nkutils/name/range/symbol",
        .data = {
            .identifier = '$',
            .source = "Dice roll gave: ${dice:[;1;byte 6;⚀;⚁;⚂;⚃;⚄;⚅]}.",
            .data = {
                { .name = "dice", .content = "uint64 6" },
                { .name = NULL }
            },
            .result = "Dice roll gave: ⚅."
        }
    },
    {
        .testpath = "/nkutils/name/range/text",
        .data = {
            .identifier = '$',
            .source = "Signal strength: ${signal:[;0;100;low;medium;high;full]}.",
            .data = {
                { .name = "signal", .content = "24" },
                { .name = NULL }
            },
            .result = "Signal strength: low."
        }
    },
    {
        .testpath = "/nkutils/name/range/plural/singular",
        .data = {
            .identifier = '$',
            .source = "${quantity} unit${quantity:[;2;2;;s]}",
            .data = {
                { .name = "quantity", .content = "1" },
                { .name = NULL }
            },
            .result = "1 unit"
        }
    },
    {
        .testpath = "/nkutils/name/range/plural/plural",
        .data = {
            .identifier = '$',
            .source = "${quantity} unit${quantity:[;2;2;;s]}",
            .data = {
                { .name = "quantity", .content = "2" },
                { .name = NULL }
            },
            .result = "2 units"
        }
    },
    {
        .testpath = "/nkutils/name/range/middle-split",
        .data = {
            .identifier = '$',
            .source = "Signal strength: ${signal:[;0;100;bad;good]}.",
            .data = {
                { .name = "signal", .content = "50" },
                { .name = NULL }
            },
            .result = "Signal strength: good."
        }
    },
    {
        .testpath = "/nkutils/name/range/double",
        .data = {
            .identifier = '$',
            .source = "${coin:[;0.0;1.0;heads;tails]}",
            .data = {
                { .name = "coin", .content = "0.6" },
                { .name = NULL }
            },
            .result = "tails"
        }
    },
    {
        .testpath = "/nkutils/name/prettify/float/width",
        .data = {
            .identifier = '$',
            .source = "${value(f4)}",
            .data = {
                { .name = "value", .content = "1" },
                { .name = NULL }
            },
            .result = "   1"
        }
    },
    {
        .testpath = "/nkutils/name/prettify/float/0-padding",
        .data = {
            .identifier = '$',
            .source = "${value(f04)}",
            .data = {
                { .name = "value", .content = "1" },
                { .name = NULL }
            },
            .result = "0001"
        }
    },
    {
        .testpath = "/nkutils/name/prettify/float/precision",
        .data = {
            .identifier = '$',
            .source = "${value(f.5)}",
            .data = {
                { .name = "value", .content = "1" },
                { .name = NULL }
            },
            .result = "1.00000"
        }
    },
    {
        .testpath = "/nkutils/name/prettify/prefixes/si/big",
        .data = {
            .identifier = '$',
            .source = "${value(p)}",
            .data = {
                { .name = "value", .content = "1000000" },
                { .name = NULL }
            },
            .result = "1M"
        }
    },
    {
        .testpath = "/nkutils/name/prettify/prefixes/si/small",
        .data = {
            .identifier = '$',
            .source = "${value(p)}",
            .data = {
                { .name = "value", .content = "0.001" },
                { .name = NULL }
            },
            .result = "1m"
        }
    },
    {
        .testpath = "/nkutils/name/prettify/prefixes/si/zero",
        .data = {
            .identifier = '$',
            .source = "${value(p)}",
            .data = {
                { .name = "value", .content = "0" },
                { .name = NULL }
            },
            .result = "0"
        }
    },
    {
        .testpath = "/nkutils/name/prettify/prefixes/si/with-precision",
        .data = {
            .identifier = '$',
            .source = "${value(p.1)}",
            .data = {
                { .name = "value", .content = "1000000" },
                { .name = NULL }
            },
            .result = "1.0M"
        }
    },
    {
        .testpath = "/nkutils/name/prettify/prefixes/si/2-precision",
        .data = {
            .identifier = '$',
            .source = "${value(p.2)}",
            .data = {
                { .name = "value", .content = "626704" },
                { .name = NULL }
            },
            .result = "626.70k"
        }
    },
    {
        .testpath = "/nkutils/name/prettify/prefixes/si/0-precision",
        .data = {
            .identifier = '$',
            .source = "${value(p.0)}",
            .data = {
                { .name = "value", .content = "626704" },
                { .name = NULL }
            },
            .result = "627k"
        }
    },
    {
        .testpath = "/nkutils/name/prettify/prefixes/binary/big",
        .data = {
            .identifier = '$',
            .source = "${value(b)}",
            .data = {
                { .name = "value", .content = "626704" },
                { .name = NULL }
            },
            .result = "612.015625Ki"
        }
    },
    {
        .testpath = "/nkutils/name/prettify/prefixes/binary/zero",
        .data = {
            .identifier = '$',
            .source = "${value(b)}",
            .data = {
                { .name = "value", .content = "0" },
                { .name = NULL }
            },
            .result = "0"
        }
    },
    {
        .testpath = "/nkutils/name/prettify/time/default",
        .data = {
            .identifier = '$',
            .source = "${timestamp(t)}",
            .data = {
                { .name = "timestamp", .content = "1519910048" },
                { .name = NULL }
            },
            .result = "Thu Mar  1 13:14:08 2018"
        }
    },
    {
        .testpath = "/nkutils/name/prettify/time/with-format",
        .data = {
            .identifier = '$',
            .source = "${timestamp(t%F %T)}",
            .data = {
                { .name = "timestamp", .content = "1519910048" },
                { .name = NULL }
            },
            .result = "2018-03-01 13:14:08"
        }
    },
    {
        .testpath = "/nkutils/name/prettify/duration/default",
        .data = {
            .identifier = '$',
            .source = "${duration(d)}",
            .data = {
                { .name = "duration", .content = "788645" },
                { .name = NULL }
            },
            .result = "1 week 2 days 3 hours 4 minutes 5 seconds"
        }
    },
    {
        .testpath = "/nkutils/name/prettify/duration/with-format",
        .data = {
            .identifier = '$',
            .source = "${duration(d%{weeks}w %{days}d %{hours(f02)}:%{minutes(f02)}:%{seconds(f02)})}",
            .data = {
                { .name = "duration", .content = "788645" },
                { .name = NULL }
            },
            .result = "1w 2d 03:04:05"
        }
    },
    {
        .testpath = "/nkutils/name/replace/full",
        .data = {
            .identifier = '$',
            .source = "You can make a ${recipe/split/cream} with ${fruit}.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'banana split'" },
                { .name = NULL }
            },
            .result = "You can make a banana cream with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/replace/missing",
        .data = {
            .identifier = '$',
            .source = "You can make a ${recipe} with ${fruit}${addition/^/ and }.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'banana split'" },
                { .name = NULL }
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/replace/capture",
        .data = {
            .identifier = '$',
            .source = "You can make a ${adjective/(.+)/(\\1) }${recipe} with ${fruit}${addition/^/ and }.",
            .data = {
                { .name = "adjective", .content = "'creamy'" },
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'banana split'" },
                { .name = "addition", .content = "'some cream'" },
                { .name = NULL }
            },
            .result = "You can make a (creamy) banana split with a banana and some cream."
        }
    },
    {
        .testpath = "/nkutils/name/replace/before-after/with",
        .data = {
            .identifier = '$',
            .source = "You can make a ${adjective/^/(/$/) }${recipe} with ${fruit}${addition/^/ and }.",
            .data = {
                { .name = "adjective", .content = "'creamy'" },
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'banana split'" },
                { .name = "addition", .content = "'some cream'" },
                { .name = NULL }
            },
            .result = "You can make a (creamy) banana split with a banana and some cream."
        }
    },
    {
        .testpath = "/nkutils/name/replace/before-after/without",
        .data = {
            .identifier = '$',
            .source = "You can make a ${adjective/^/(/$/) }${recipe} with ${fruit}${addition/^/ and }.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'banana split'" },
                { .name = NULL }
            },
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/replace/remove",
        .data = {
            .identifier = '$',
            .source = "You can make a ${recipe/ split} with ${fruit}.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'banana split'" },
                { .name = NULL }
            },
            .result = "You can make a banana with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/replace/multiple",
        .data = {
            .identifier = '$',
            .source = "You can make ${recipe/a banana/an apple pie/ split} with ${fruit/.+/apples}.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'a banana split'" },
                { .name = NULL }
            },
            .result = "You can make an apple pie with apples."
        }
    },
    {
        .testpath = "/nkutils/name/replace/braces/paired",
        .data = {
            .identifier = '$',
            .source = "You can make a ${adjective/.{2}$/y/^/(/$/) }${recipe} with ${fruit}${addition/\\{//\\}//^/ and }.",
            .data = {
                { .name = "adjective", .content = "'creamed'" },
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'banana split'" },
                { .name = "addition", .content = "'some cream{}'" },
                { .name = NULL }
            },
            .result = "You can make a (creamy) banana split with a banana and some cream."
        }
    },
    {
        .testpath = "/nkutils/name/replace/braces/opening",
        .data = {
            .identifier = '$',
            .source = "You can make a ${adjective/.{2}$/y/^/(/$/) }${recipe} with ${fruit}${addition/\\{//^/ and }.",
            .data = {
                { .name = "adjective", .content = "'creamed'" },
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'banana split'" },
                { .name = "addition", .content = "'some cream{'" },
                { .name = NULL }
            },
            .result = "You can make a (creamy) banana split with a banana and some cream."
        }
    },
    {
        .testpath = "/nkutils/name/replace/braces/closing",
        .data = {
            .identifier = '$',
            .source = "You can make a ${adjective/.{2}$/y/^/(/$/) }${recipe} with ${fruit}${addition/\\}//^/ and }.",
            .data = {
                { .name = "adjective", .content = "'creamed'" },
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'banana split'" },
                { .name = "addition", .content = "'some cream}'" },
                { .name = NULL }
            },
            .result = "You can make a (creamy) banana split with a banana and some cream."
        }
    },
    {
        .testpath = "/nkutils/name/replace/escaping/backslash",
        .data = {
            .identifier = '$',
            .source = "You can make a ${adjective/^/(/$/) /\\\\}${recipe} with ${fruit}${addition/^/ and }.",
            .data = {
                { .name = "adjective", .content = "'creamy\\\\'" },
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'banana split'" },
                { .name = "addition", .content = "'some cream'" },
                { .name = NULL }
            },
            .result = "You can make a (creamy) banana split with a banana and some cream."
        }
    },
    {
        .testpath = "/nkutils/name/replace/escaping/forwardslash/1",
        .data = {
            .identifier = '$',
            .source = "${data/\\/}",
            .data = {
                { .name = "data", .content = "'/'" },
                { .name = NULL }
            },
            .result = ""
        }
    },
    {
        .testpath = "/nkutils/name/replace/escaping/forwardslash/2",
        .data = {
            .identifier = '$',
            .source = "${data/a/\\/}",
            .data = {
                { .name = "data", .content = "'a'" },
                { .name = NULL }
            },
            .result = "/"
        }
    },
    {
        .testpath = "/nkutils/name/replace/escaping/forwardslash/3",
        .data = {
            .identifier = '$',
            .source = "${data/a/\\//b/x}",
            .data = {
                { .name = "data", .content = "'ab'" },
                { .name = NULL }
            },
            .result = "/x"
        }
    },
    {
        .testpath = "/nkutils/name/replace/escaping/right-curly-bracket/2",
        .data = {
            .identifier = '$',
            .source = "${data/a/\\}}",
            .data = {
                { .name = "data", .content = "'a'" },
                { .name = NULL }
            },
            .result = "}"
        }
    },
    {
        .testpath = "/nkutils/name/replace/recurse/with",
        .data = {
            .identifier = '$',
            .source = "I want to eat ${recipe/an apple/${fruit}}.",
            .data = {
                { .name = "recipe", .content = "'an apple pie'" },
                { .name = "fruit", .content = "'a blackberry'" },
                { .name = NULL }
            },
            .result = "I want to eat a blackberry pie."
        }
    },
    {
        .testpath = "/nkutils/name/old/before-after",
        .data = {
            .identifier = '$',
            .source = "You can make a ${(<adjective>) }${recipe} with ${fruit}${ and <addition}.",
            .data = {
                { .name = "fruit", .content = "'a banana'" },
                { .name = "recipe", .content = "'banana split'" },
                { .name = NULL }
            },
            .result = "You can make a ${(<adjective>) }banana split with a banana${ and <addition}."
        }
    },
    {
        .testpath = "/nkutils/name/identifier/double-escape",
        .data = {
            .identifier = '$',
            .source = "echo $${PATH}",
            .data = {
                { .name = NULL }
            },
            .result = "echo ${PATH}"
        }
    },
    {
        .testpath = "/nkutils/name/identifier/non-dollar",
        .data = {
            .identifier = '%',
            .source = "Some %{variable}",
            .data = {
                { .name = "variable", .content = "'value'" },
                { .name = NULL }
            },
            .result = "Some value"
        }
    },
    {
        .testpath = "/nkutils/name/identifier/none",
        .data = {
            .identifier = '\0',
            .source = "Some {variable}",
            .data = {
                { .name = "variable", .content = "'value'" },
                { .name = NULL }
            },
            .result = "Some value"
        }
    },
};

static GVariant *
_nk_format_string_tests_callback(const gchar *name, guint64 value, gpointer user_data)
{
    NkFormatStringTestData *test_data = user_data;
    NkFormatStringTestDataData *data;
    g_assert_cmpuint(value, ==, 0);
    for ( data = test_data->data ; data->name != NULL ; ++data )
    {
        if ( g_strcmp0(name, data->name) == 0 )
            return g_variant_parse(NULL, data->content, NULL, NULL, NULL);
    }
    return NULL;
}

static void
_nk_format_string_tests_func(gconstpointer user_data)
{
    NkFormatStringTestData *data = (NkFormatStringTestData *) user_data;
    NkFormatString *format_string;
    GError *error = NULL;

    format_string = nk_format_string_parse(g_strdup(data->source), data->identifier, &error);
    if ( data->result == NULL )
    {
        g_assert_null(format_string);
        g_assert_error(error, NK_FORMAT_STRING_ERROR, data->error);
        return;
    }
    g_assert_nonnull(format_string);
    g_assert_no_error(error);
    g_assert_nonnull(nk_format_string_ref(format_string));

    gchar *result;
    result = nk_format_string_replace(format_string, _nk_format_string_tests_callback, data);

    g_assert_cmpstr(result, ==, data->result);

    nk_format_string_unref(format_string);
    nk_format_string_unref(format_string);
}

typedef enum {
    TOKEN_FRUIT,
    TOKEN_RECIPE,
    TOKEN_VALUE,
    _TOKEN_SIZE
} NkFormatStringEnumTokens;

static const gchar * const _nk_format_string_enum_tests_tokens[_TOKEN_SIZE] = {
    [TOKEN_FRUIT]  = "fruit",
    [TOKEN_RECIPE] = "recipe",
    [TOKEN_VALUE] = "value",
};

typedef struct {
    gunichar identifier;
    const gchar *source;
    gchar *data[_TOKEN_SIZE];
    guint64 used_tokens;
    gint error;
    const gchar *result;
} NkFormatStringEnumTestData;

static const struct {
    const gchar *testpath;
    NkFormatStringEnumTestData data;
} _nk_format_string_enum_tests_list[] = {
    {
        .testpath = "/nkutils/name/enum/basic",
        .data = {
            .identifier = '$',
            .source = "You can make ${recipe} with ${fruit}.",
            .data = {
                [TOKEN_FRUIT]  = "'a banana'",
                [TOKEN_RECIPE] = "'a banana split'",
            },
            .used_tokens = (1 << TOKEN_FRUIT) | (1 << TOKEN_RECIPE),
            .result = "You can make a banana split with a banana."
        }
    },
    {
        .testpath = "/nkutils/name/enum/nested",
        .data = {
            .identifier = '$',
            .source = "I want to eat ${recipe:+${fruit} ${recipe}}.",
            .data = {
                [TOKEN_FRUIT]  = "'an apple'",
                [TOKEN_RECIPE] = "'pie'",
            },
            .used_tokens = (1 << TOKEN_FRUIT) | (1 << TOKEN_RECIPE),
            .result = "I want to eat an apple pie."
        }
    },
    {
        .testpath = "/nkutils/name/enum/fallback",
        .data = {
            .identifier = '$',
            .source = "I want to eat ${fruit} ${recipe:-pie}.",
            .data = {
                [TOKEN_FRUIT]  = "'an apple'",
            },
            .used_tokens = (1 << TOKEN_FRUIT) | (1 << TOKEN_RECIPE),
            .result = "I want to eat an apple pie."
        }
    },
    {
        .testpath = "/nkutils/name/enum/substitute",
        .data = {
            .identifier = '$',
            .source = "I want to eat ${fruit:+a fruit}.",
            .data = {
                [TOKEN_FRUIT]  = "'an apple'",
            },
            .used_tokens = (1 << TOKEN_FRUIT),
            .result = "I want to eat a fruit."
        }
    },
    {
        .testpath = "/nkutils/name/enum/anti-substitute",
        .data = {
            .identifier = '$',
            .source = "I want to eat${fruit:! a fruit}.",
            .data = {
                [TOKEN_FRUIT]  = "'an apple'",
            },
            .used_tokens = (1 << TOKEN_FRUIT),
            .result = "I want to eat."
        }
    },
    {
        .testpath = "/nkutils/name/enum/prettify/float",
        .data = {
            .identifier = '$',
            .source = "${value(f.2)}",
            .data = {
                [TOKEN_VALUE]  = "1.5555",
            },
            .used_tokens = (1 << TOKEN_VALUE),
            .result = "1.56"
        }
    },
    {
        .testpath = "/nkutils/name/enum/prettify/prefixes/si",
        .data = {
            .identifier = '$',
            .source = "${value(p)}",
            .data = {
                [TOKEN_VALUE]  = "1000",
            },
            .used_tokens = (1 << TOKEN_VALUE),
            .result = "1k"
        }
    },
    {
        .testpath = "/nkutils/name/enum/prettify/prefixes/binary",
        .data = {
            .identifier = '$',
            .source = "${value(b)}",
            .data = {
                [TOKEN_VALUE]  = "1024",
            },
            .used_tokens = (1 << TOKEN_VALUE),
            .result = "1Ki"
        }
    },
    {
        .testpath = "/nkutils/name/enum/regex",
        .data = {
            .identifier = '$',
            .source = "I want to eat ${fruit/an apple/a banana} ${recipe}.",
            .data = {
                [TOKEN_FRUIT]  = "'an apple'",
                [TOKEN_RECIPE]  = "'split'",
            },
            .used_tokens = (1 << TOKEN_FRUIT) | (1 << TOKEN_RECIPE),
            .result = "I want to eat a banana split."
        }
    },
    {
        .testpath = "/nkutils/name/enum/wrong/regex",
        .data = {
            .identifier = '$',
            .source = "You can make a ${recipe/[} with ${fruit}.",
            .error = NK_FORMAT_STRING_ERROR_REGEX,
        }
    },
    {
        .testpath = "/nkutils/name/enum/wrong/name",
        .data = {
            .identifier = '$',
            .source = "You can make a ${recipe} with ${fruit} and ${addition}.",
            .error = NK_FORMAT_STRING_ERROR_UNKNOWN_TOKEN,
        }
    },
};

static GVariant *
_nk_format_string_enum_tests_callback(const gchar *name, guint64 value, gpointer user_data)
{
    const gchar * const *data = user_data;
    g_assert_cmpstr(name, ==, _nk_format_string_enum_tests_tokens[value]);
    if ( data[value] == NULL )
        return NULL;
    return g_variant_parse(NULL, data[value], NULL, NULL, NULL);
}

static void
_nk_format_string_enum_tests_func(gconstpointer user_data)
{
    NkFormatStringEnumTestData *data = (NkFormatStringEnumTestData *) user_data;
    NkFormatString *format_string;
    guint64 used_tokens;
    GError *error = NULL;

    format_string = nk_format_string_parse_enum(g_strdup(data->source), data->identifier, _nk_format_string_enum_tests_tokens, _TOKEN_SIZE, &used_tokens, &error);
    if ( data->result == NULL )
    {
        g_assert_null(format_string);
        g_assert_error(error, NK_FORMAT_STRING_ERROR, data->error);
        return;
    }
    g_assert_no_error(error);
    g_assert_nonnull(format_string);
    if ( data->used_tokens != 0 )
        g_assert_cmpuint(used_tokens, ==, data->used_tokens);

    gchar *result;
    result = nk_format_string_replace(format_string, _nk_format_string_enum_tests_callback, data->data);

    g_assert_cmpstr(result, ==, data->result);

    nk_format_string_unref(format_string);
}

int
main(int argc, char *argv[])
{
    setlocale(LC_ALL, "C");

    g_setenv("LANG", "C", TRUE);
    g_setenv("TZ", "UTC", TRUE);

    g_test_init(&argc, &argv, NULL);

    g_test_set_nonfatal_assertions();

    gsize i;
    for ( i = 0 ; i < G_N_ELEMENTS(_nk_format_string_tests_list) ; ++i )
        g_test_add_data_func(_nk_format_string_tests_list[i].testpath, &_nk_format_string_tests_list[i].data, _nk_format_string_tests_func);

    for ( i = 0 ; i < G_N_ELEMENTS(_nk_format_string_enum_tests_list) ; ++i )
        g_test_add_data_func(_nk_format_string_enum_tests_list[i].testpath, &_nk_format_string_enum_tests_list[i].data, _nk_format_string_enum_tests_func);

    return g_test_run();
}
