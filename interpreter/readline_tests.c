#include <cgreen/cgreen.h>
#include <cgreen/mocks.h>

#include "readline.h"

#include "options.h"


/* Mocked modules */
#include "syserr.mock"
#include "output.mock"
#include "converter.mock"

/* Need this just because instance_tests.c uses real set, so other isolated tests must link it too */
#include "instance.mock"

ssize_t mocked_read(int fd, char *buf, size_t nbytes) {
    return mock(fd, buf, nbytes);
}

ssize_t mocked_write(int fd, char *buf, size_t nbytes) {
    return mock(fd, buf, nbytes);
}

Describe(Readline);
BeforeEach(Readline) {
    never_expect(doBeep);
    always_expect(mocked_write, will_return(1));
}
AfterEach(Readline) {}


static void expect_newline(void) {
    static char newline = '\n';
    expect(mocked_read,
           will_set_contents_of_parameter(buf, &newline, sizeof(char)),
           will_return(1));
}

static void expect_a(void) {
    static char stubbed_a = 'a';
    expect(mocked_read,
           will_set_contents_of_parameter(buf, &stubbed_a, sizeof(char)),
           will_return(1));
}

static void expect_b(void) {
    static char stubbed_b = 'b';
    expect(mocked_read,
           will_set_contents_of_parameter(buf, &stubbed_b, sizeof(char)),
           will_return(1));
}

static void expect_c(void) {
    static char stubbed_c = 'c';
    expect(mocked_read,
           will_set_contents_of_parameter(buf, &stubbed_c, sizeof(char)),
           will_return(1));
}


Ensure(Readline, can_read_some_ascii_characters) {
    char buffer[100];

    /* Expect to read 'abc' ... */
    expect_a();
    expect_b();
    expect_c();

    /* Enter */
    expect_newline();

    expect(ensureInternalEncoding,
           when(string, is_equal_to_string("abc")),
           will_return(strdup("abc"))); /* Because it should be malloc'ed */

    readline(buffer);
    assert_that(buffer, is_equal_to_string("abc"));
}


static void expect_delBwd(void) {
    static char delBwd = '\x08';
    expect(mocked_read,
           will_set_contents_of_parameter(buf, &delBwd, sizeof(char)),
           will_return(1));
}


Ensure(Readline, can_delete_last_ascii_character) {
    char buffer[100];

    /* Expect to read 'abc' ... */
    expect_a();
    expect_b();
    expect_c();

    /* Now delete last */
    expect_delBwd();

    expect_newline();

    expect(ensureInternalEncoding,
           when(string, is_equal_to_string("ab")),
           will_return(strdup("ab"))); /* Because it should be malloc'ed */

    readline(buffer);

    assert_that(buffer, is_equal_to_string("ab"));
}


static void expect_leftArrow(void) {
    static char escHook = '\x1b';
    static char arrowHook = '\x5b';
    static char leftArrow = '\x44';
    expect(mocked_read,
           will_set_contents_of_parameter(buf, &escHook, sizeof(char)),
           will_return(1));
    expect(mocked_read,
           will_set_contents_of_parameter(buf, &arrowHook, sizeof(char)),
           will_return(1));
    expect(mocked_read,
           will_set_contents_of_parameter(buf, &leftArrow, sizeof(char)),
           will_return(1));
}


Ensure(Readline, can_delete_an_ascii_character_in_the_middle) {
    char buffer[100];

    /* Expect to read 'a', 'b', 'c' ... */
    expect_a();
    expect_b();
    expect_c();

    /* Move left */
    expect_leftArrow();

    /* Now delete the b */
    expect_delBwd();

    /* ... and newline... */
    expect_newline();

    expect(ensureInternalEncoding,
           when(string, is_equal_to_string("ac")),
           will_return(strdup("ac"))); /* Because it should be malloc'ed */

    readline(buffer);

    assert_that(buffer, is_equal_to_string("ac"));
}

Ensure(Readline, can_insert_an_ascii_character_in_the_middle) {
    char buffer[100];

    /* Expect to read 'a', 'c' ... */
    expect_a();
    expect_c();

    /* Move left */
    expect_leftArrow();

    expect_b();

    /* ... and newline... */
    expect_newline();

    expect(ensureInternalEncoding,
           when(string, is_equal_to_string("abc")),
           will_return(strdup("abc"))); /* Because it should be malloc'ed */

    readline(buffer);

    assert_that(buffer, is_equal_to_string("abc"));
}

static void expect_aring(void) {
    static uchar stubbed_aring[2] = {0xC3, 0xA5}; // 'å';
    expect(mocked_read,
           will_set_contents_of_parameter(buf, &stubbed_aring[0], sizeof(char)),
           will_return(1));
    expect(mocked_read,
           will_set_contents_of_parameter(buf, &stubbed_aring[1], sizeof(char)),
           will_return(1));
}

static void expect_adiaeresis(void) {
    static uchar stubbed_adiaeresis[2] = {0xC3, 0xA4}; // 'ä';
    expect(mocked_read,
           will_set_contents_of_parameter(buf, &stubbed_adiaeresis[0], sizeof(char)),
           will_return(1));
    expect(mocked_read,
           will_set_contents_of_parameter(buf, &stubbed_adiaeresis[1], sizeof(char)),
           will_return(1));
}

static void expect_odiaeresis(void) {
    static uchar stubbed_odiaeresis[2] = {0xC3, 0xB6}; // 'ö';
    expect(mocked_read,
           will_set_contents_of_parameter(buf, &stubbed_odiaeresis[0], sizeof(char)),
           will_return(1));
    expect(mocked_read,
           will_set_contents_of_parameter(buf, &stubbed_odiaeresis[1], sizeof(char)),
           will_return(1));
}


Ensure(Readline, can_read_some_utf8_characters) {
    char buffer[100];

    encodingOption = ENCODING_UTF;

    /* Expect to read 3 UTF-chars, which is actually 6 bytes ... */
    expect_aring();
    expect_adiaeresis();
    expect_odiaeresis();

    expect_newline();

    expect(ensureInternalEncoding,
           when(string, is_equal_to_string("\xC3\xA5\xC3\xA4\xC3\xB6")),
           will_return(strdup("\xE5\xE4\xF6"))); /* Because it should be malloc'ed */

    readline(buffer);

    assert_that(buffer, is_equal_to_string("\xE5\xE4\xF6"));
}


Ensure(Readline, can_delete_last_utf8_character) {
    char buffer[100];

    encodingOption = ENCODING_UTF;

    /* Expect to read 3 UTF-chars, which is actually 6 bytes ... */
    expect_aring();
    expect_adiaeresis();
    expect_odiaeresis();

    /* ... and delete the last character */
    expect_delBwd();

    /* Enter */
    expect_newline();

    expect(ensureInternalEncoding,
           when(string, is_equal_to_string("\xC3\xA5\xC3\xA4")), /* åä */
           will_return(strdup("\xE5\xE4"))); /* Because it should be malloc'ed */

    readline(buffer);

    assert_that(buffer, is_equal_to_string("\xE5\xE4"));
}


Ensure(Readline, can_delete_an_utf8_character_in_the_middle) {
    char buffer[100];

    encodingOption = ENCODING_UTF;

    /* Expect to read 3 UTF-chars, which is actually 6 bytes ... */
    expect_aring();
    expect_adiaeresis();
    expect_odiaeresis();

    /* backup one UTF-8 character */
    expect_leftArrow();

    /* ... and delete the last character */
    expect_delBwd();

    /* Enter */
    expect_newline();

    expect(ensureInternalEncoding,
           when(string, is_equal_to_string("\xC3\xA5\xC3\xB6")), /* åö */
           will_return(strdup("\xE5\xF6"))); /* Because it should be malloc'ed */

    readline(buffer);
    assert_that(buffer, is_equal_to_string("\xE5\xF6"));
}


Ensure(Readline, can_insert_an_utf8_character_in_the_middle) {
    char buffer[100];

    encodingOption = ENCODING_UTF;

    /* Type two UTF-8 characters */
    expect_aring();
    expect_odiaeresis();

    /* backup one UTF-8 character */
    expect_leftArrow();

    /* ... and type another UTF-8 character */
    expect_adiaeresis();

    /* Enter */
    expect_newline();

    expect(ensureInternalEncoding,
           when(string, is_equal_to_string("\xC3\xA5\xC3\xA4\xC3\xB6")), /* åäö */
           will_return(strdup("\xE5\xE4\xF6"))); /* Because it should be malloc'ed */

    readline(buffer);
    assert_that(buffer, is_equal_to_string("\xE5\xE4\xF6"));
}

extern int ustrlen(uchar *utf_string);

Ensure(Readline, can_count_utf_chars) {
    uchar three_ascii[] = "abc";
    assert_that(ustrlen(three_ascii), is_equal_to(3));

    uchar four_ascii[] = "abcd";
    assert_that(ustrlen(four_ascii), is_equal_to(4));

    uchar one_utf[] = "ö";
    assert_that(ustrlen(one_utf), is_equal_to(1));

    uchar two_utf[] = "öñ";
    assert_that(ustrlen(two_utf), is_equal_to(2));

    uchar mixed[] = "Aöabñ4";
    assert_that(ustrlen(mixed), is_equal_to(6));
}
