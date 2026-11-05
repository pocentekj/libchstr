#include <check.h>
#include <stdlib.h>

#include "src/chstr.h"

// clang-format off

START_TEST(test_starts_with)
{
    ck_assert(ch_starts_with("/dev/mapper/root", "/dev"));
    ck_assert(!ch_starts_with("/dev/mapper/root", "dev"));
}
END_TEST

START_TEST(test_string_case)
{
    char *test_str = strdup("Zażółć gęślą żółć");
    char *lower = ch_string_to_lowercase(test_str);
    ck_assert(lower);
    ck_assert_str_eq(lower, "zażółć gęślą żółć");
    char *upper = ch_string_to_uppercase(test_str);
    ck_assert(upper);
    ck_assert_str_eq(upper, "ZAŻÓŁĆ GĘŚLĄ ŻÓŁĆ");
    free(upper);
    free(lower);
    free(test_str);
}
END_TEST

START_TEST(test_string_width)
{
    ck_assert(ch_string_width("hello") == 5);
    ck_assert(ch_string_width("なかの あずさ") == 13);
    ck_assert(ch_string_width("Zażółć gęślą żółć") == 17);
}
END_TEST

START_TEST(test_string_center)
{
    char *test_str = strdup("🐾 中野 梓");
    char *centered = NULL;
    ck_assert(ch_string_center(test_str, 24, &centered) > 0);
    ck_assert_str_eq(centered, "       🐾 中野 梓       ");
    free(centered);
    free(test_str);
}
END_TEST

START_TEST(test_string_truncate)
{
    char *test_str = NULL;
    bool truncated = false;

    test_str = strdup("Hello, World!");
    truncated = ch_string_truncate(test_str, 5);
    ck_assert(truncated);
    ck_assert_str_eq(test_str, "Hello");
    free(test_str);

    test_str = strdup("Hello, World!");
    truncated = ch_string_truncate(test_str, 128);
    ck_assert(!truncated);
    ck_assert_str_eq(test_str, "Hello, World!");
    free(test_str);

    test_str = strdup("なかの あずさ");
    truncated = ch_string_truncate(test_str, 5);
    ck_assert(truncated);
    ck_assert_str_eq(test_str, "なか");
    free(test_str);
}
END_TEST

START_TEST(test_parse_double)
{
    double val = 0.0;
    bool ok = false;

    ok = ch_parse_double("0.025", &val);
    ck_assert(ok);
    ck_assert_double_eq(val, 0.025);

    ok = ch_parse_double("invalid", &val);
    ck_assert(!ok);
}
END_TEST

START_TEST(test_parse_int)
{
    int val = 0;
    bool ok = false;

    ok = ch_parse_int("42", &val);
    ck_assert(ok);
    ck_assert_int_eq(val, 42);

    ok = ch_parse_int("invalid", &val);
    ck_assert(!ok);

    ok = ch_parse_int("10m", &val);
    ck_assert(!ok);

    ok = ch_parse_int("0.025", &val);
    ck_assert(!ok);
}
END_TEST

START_TEST(test_string_from_file)
{
    const char * test_str = "平沢 唯（ひらさわ ゆい）\n田井中 律（たいなか りつ）\n秋山 澪（あきやま みお）\n琴吹 紬（ことぶき つむぎ）\n中野 梓（なかの あずさ）\n";
    char *contents_1 = ch_string_from_file("../testfile.txt", NULL);
    ck_assert(contents_1 != NULL);
    ck_assert_str_eq(contents_1, test_str);
    free(contents_1);
    char *contents_2 = ch_string_from_file("does-not-exist.txt", NULL);
    ck_assert(contents_2 == NULL);
    free(contents_2);
}
END_TEST

START_TEST(test_read_lines)
{
    string_array *lines = ch_string_array_new();
    bool ok = ch_read_lines("../testfile.txt", lines);

    ck_assert(ok);
    ck_assert(ch_string_array_len(lines) == 5);
    ck_assert_str_eq(ch_string_array_at(lines, 0), "平沢 唯（ひらさわ ゆい）");

    ch_string_array_destroy(lines);
}
END_TEST

// clang-format on

Suite *dysk_c_suite(void) {
    Suite *s = suite_create("chstr");
    TCase *tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_starts_with);
    tcase_add_test(tc_core, test_string_case);
    tcase_add_test(tc_core, test_string_width);
    tcase_add_test(tc_core, test_string_center);
    tcase_add_test(tc_core, test_string_truncate);
    tcase_add_test(tc_core, test_parse_int);
    tcase_add_test(tc_core, test_parse_double);
    tcase_add_test(tc_core, test_string_from_file);
    tcase_add_test(tc_core, test_read_lines);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(void) {
    Suite *s = dysk_c_suite();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_VERBOSE);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
