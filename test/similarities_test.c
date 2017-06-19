//
// Created by Paul Kiman on 18.06.17.
//
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include "similarities_test.h"
#include "../similarities.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
static const char *lib_filename = "similarities.dll";
#else
static const char *lib_filename = "similarities.so";
#endif

void* lib_handle = NULL;

int tests_run = 0;

static char * strip_w_test_1() {
    if(lib_handle != NULL) {
        printf("Testing => %s\n", __FUNCTION__);

        char *(*strip_w)() = dlsym(lib_handle, "_strip_w");
        char *testString1 = "          This is           \r               a dirty test  string   with many  whitespaces   \n  and newlines    \n\r    ";
        unsigned long len_testString1 = strlen(testString1);
        char *result1 = strip_w(testString1, len_testString1);
        //printf("%lu %lu %s\n", len_testString1, strlen(result1), result1);
        mu_assert("Error, strip_w_test_1 => testString1 length - expected 63", strlen(result1) == 63);
    }

    return 0;
}

static char * strip_w_test_2() {
    if(lib_handle != NULL) {
        printf("Testing => %s\n", __FUNCTION__);

        char *(*strip_w)() = dlsym(lib_handle, "_strip_w");
        char *testString2 = "     \n       \r         \n \r    ";
        unsigned long len_testString2 = strlen(testString2);
        char *result2 = strip_w(testString2, len_testString2);
        //printf("%lu %lu %s\n", len_testString2, strlen(result2), result2);
        mu_assert("Error, strip_w_test_2 => testString2 length - expected 0", strlen(result2) == 0);
    }

    return 0;
}

static char * strip_w_test_3() {
    if(lib_handle != NULL) {
        printf("Testing => %s\n", __FUNCTION__);

        char *(*strip_w)() = dlsym(lib_handle, "_strip_w");
        char *testString3 = "\n       \r         \n \r    ";
        unsigned long len_testString3 = strlen(testString3);
        char *result3 = strip_w(testString3, len_testString3);
        //printf("%lu %lu %s\n", len_testString3, strlen(result3), result3);
        mu_assert("Error, strip_w_test_3 => testString3 length - expected 0", strlen(result3) == 0);
    }

    return 0;
}

static char * levenshtein_k_core_test() {
    if(lib_handle != NULL) {
        printf("Testing => %s\n", __FUNCTION__);

        longlong (*levenshtein_k_core)() = dlsym(lib_handle, "_levenshtein_k_core");
        char *testString1 = "This is a test string with many whitespaces and newlines";
        char *testString2 = "This is not a test string with many whitespaces and newlines";
        longlong result3 = levenshtein_k_core(testString1, strlen(testString1), testString2, strlen(testString2), 255);
        mu_assert("Error, levenshtein_k_core_test_3 => testString3 length - expected 4", result3 == 4);
    }

    return 0;
}

static char * levenshtein_test() {

    if(lib_handle != NULL) {
        printf("Testing => %s\n", __FUNCTION__);

        char *testString1 = "This is a test string with many whitespaces and newlines";
        char *testString2 = "This is not a test string with many whitespaces and newlines";


        my_bool (*levenshtein_init)() = dlsym(lib_handle, "levenshtein_init");
        longlong (*levenshtein)() = dlsym(lib_handle, "levenshtein");
        void(*levenshtein_deinit)() = dlsym(lib_handle, "levenshtein_deinit");

        UDF_INIT *init = (UDF_INIT *) malloc(sizeof(UDF_INIT));
        UDF_ARGS *args = (UDF_ARGS *) malloc(sizeof(UDF_ARGS));
        args->arg_type = (enum Item_result *) malloc(sizeof(enum Item_result)*2);
        args->lengths = (long unsigned int *) malloc(sizeof(long unsigned int)*2);
        char *message = (char *) malloc(sizeof(char)*MYSQL_ERRMSG_SIZE);
        char *error = (char *) malloc(sizeof(char));
        char *is_null = (char *) malloc(sizeof(char));

        args->arg_type[0] = STRING_RESULT;
        args->arg_type[1] = STRING_RESULT;
        args->lengths[0] = strlen(testString1);
        args->lengths[1] = strlen(testString2);
        args->arg_count = 2;
        is_null[0] = '\0';
        error[0] = '\0';
        args->args = (char **) malloc(sizeof(char *)*2);
        args->args[0] = (char *) malloc(sizeof(char)*(args->lengths[0]));
        args->args[0] = testString1;
        args->args[1] = (char *) malloc(sizeof(char)*(args->lengths[1]));
        args->args[1] = testString2;

        my_bool ret = levenshtein_init(init, args, message);
        mu_assert("Error, levenshtein_test => levenshtein_init - expected 0", ret == 0);

        longlong result = levenshtein(init, args, is_null, error);
        mu_assert("Error, levenshtein_test => levenshtein - expected 4", result == 4);


        levenshtein_deinit(init);

        free(args->args);
        free(is_null);
        free(error);
        free(message);
        free(args->arg_type);
        free(args->lengths);
        free(args);
        free(init);
    }

    return 0;
}

static char * levenshtein_k_test() {
    if(lib_handle != NULL) {
        printf("Testing => %s\n", __FUNCTION__);

        long long limit_arg = 255;
        char *testString1 = "Test String with many white spaces";
        char *testString2 = "This is not a test string with many whitespaces and newlines";


        my_bool (*levenshtein_k_init)() = dlsym(lib_handle, "levenshtein_k_init");
        longlong (*levenshtein_k)() = dlsym(lib_handle, "levenshtein_k");
        void(*levenshtein_k_deinit)() = dlsym(lib_handle, "levenshtein_k_deinit");

        UDF_INIT *init = (UDF_INIT *) malloc(sizeof(UDF_INIT));
        UDF_ARGS *args = (UDF_ARGS *) malloc(sizeof(UDF_ARGS));
        args->arg_type = (enum Item_result *) malloc(sizeof(enum Item_result)*3);
        args->lengths = (long unsigned int *) malloc(sizeof(long unsigned int)*2);
        char *message = (char *) malloc(sizeof(char)*MYSQL_ERRMSG_SIZE);
        char *error = (char *) malloc(sizeof(char));
        char *is_null = (char *) malloc(sizeof(char));

        long long *limit_arg_ptr = &limit_arg;

        args->arg_type[0] = STRING_RESULT;
        args->arg_type[1] = STRING_RESULT;
        args->arg_type[2] = INT_RESULT;
        args->lengths[0] = strlen(testString1);
        args->lengths[1] = strlen(testString2);
        args->arg_count = 3;
        is_null[0] = '\0';
        error[0] = '\0';
        args->args = (char **) malloc(sizeof(char *)*3);
        args->args[0] = (char *) malloc(sizeof(char)*(args->lengths[0]));
        args->args[0] = testString1;
        args->args[1] = (char *) malloc(sizeof(char)*(args->lengths[1]));
        args->args[1] = testString2;
        args->args[2] = (char *) limit_arg_ptr;

        my_bool ret = levenshtein_k_init(init, args, message);
        mu_assert("Error, levenshtein_k_test => levenshtein_k_init - expected 0", ret == 0);

        longlong result = levenshtein_k(init, args, is_null, error);
        mu_assert("Error, levenshtein_k_test => levenshtein_k - expected 4", result == 29);

        levenshtein_k_deinit(init);

        free(args->args);
        free(is_null);
        free(error);
        free(message);
        free(args->arg_type);
        free(args->lengths);
        free(args);
        free(init);
    }

    return 0;
}

static char * levenshtein_substring_k_test1() {
    if(lib_handle != NULL) {
        printf("Testing => %s\n", __FUNCTION__);

        long long limit_arg = 255;
        char *testString1 = "Test String     with many white       spaces    ";
        char *testString2 = "This is not a test string with many whitespaces and newlines";


        my_bool (*levenshtein_substring_k_init)() = dlsym(lib_handle, "levenshtein_substring_k_init");
        longlong (*levenshtein_substring_k)() = dlsym(lib_handle, "levenshtein_substring_k");
        void(*levenshtein_substring_k_deinit)() = dlsym(lib_handle, "levenshtein_substring_k_deinit");

        UDF_INIT *init = (UDF_INIT *) malloc(sizeof(UDF_INIT));
        UDF_ARGS *args = (UDF_ARGS *) malloc(sizeof(UDF_ARGS));
        args->arg_type = (enum Item_result *) malloc(sizeof(enum Item_result)*3);
        args->lengths = (long unsigned int *) malloc(sizeof(long unsigned int)*2);
        char *message = (char *) malloc(sizeof(char)*MYSQL_ERRMSG_SIZE);
        char *error = (char *) malloc(sizeof(char));
        char *is_null = (char *) malloc(sizeof(char));

        long long *limit_arg_ptr = &limit_arg;

        args->arg_type[0] = STRING_RESULT;
        args->arg_type[1] = STRING_RESULT;
        args->arg_type[2] = INT_RESULT;
        args->lengths[0] = strlen(testString1);
        args->lengths[1] = strlen(testString2);
        args->arg_count = 3;
        is_null[0] = '\0';
        error[0] = '\0';
        args->args = (char **) malloc(sizeof(char *)*3);
        args->args[0] = (char *) malloc(sizeof(char)*(args->lengths[0]));
        args->args[0] = testString1;
        args->args[1] = (char *) malloc(sizeof(char)*(args->lengths[1]));
        args->args[1] = testString2;
        args->args[2] = (char *) limit_arg_ptr;

        my_bool ret = levenshtein_substring_k_init(init, args, message);
        mu_assert("Error, levenshtein_substring_k_test1 => levenshtein_substring_k_init - expected 0", ret == 0);

        longlong result = levenshtein_substring_k(init, args, is_null, error);
        mu_assert("Error, levenshtein_substring_k_test1 => levenshtein_substring_k - expected 4", result == 4);


        levenshtein_substring_k_deinit(init);

        free(args->args);
        free(is_null);
        free(error);
        free(message);
        free(args->arg_type);
        free(args->lengths);
        free(args);
        free(init);
    }

    return 0;
}

static char * levenshtein_substring_k_test2() {
    if(lib_handle != NULL) {
        printf("Testing => %s\n", __FUNCTION__);

        long long limit_arg = 255;
        char *testString1 = "Levenhstein";
        char *testString2 = "This is a long string Levenshtein";


        my_bool (*levenshtein_substring_k_init)() = dlsym(lib_handle, "levenshtein_substring_k_init");
        longlong (*levenshtein_substring_k)() = dlsym(lib_handle, "levenshtein_substring_k");
        void(*levenshtein_substring_k_deinit)() = dlsym(lib_handle, "levenshtein_substring_k_deinit");

        UDF_INIT *init = (UDF_INIT *) malloc(sizeof(UDF_INIT));
        UDF_ARGS *args = (UDF_ARGS *) malloc(sizeof(UDF_ARGS));
        args->arg_type = (enum Item_result *) malloc(sizeof(enum Item_result)*3);
        args->lengths = (long unsigned int *) malloc(sizeof(long unsigned int)*2);
        char *message = (char *) malloc(sizeof(char)*MYSQL_ERRMSG_SIZE);
        char *error = (char *) malloc(sizeof(char));
        char *is_null = (char *) malloc(sizeof(char));

        long long *limit_arg_ptr = &limit_arg;

        args->arg_type[0] = STRING_RESULT;
        args->arg_type[1] = STRING_RESULT;
        args->arg_type[2] = INT_RESULT;
        args->lengths[0] = strlen(testString1);
        args->lengths[1] = strlen(testString2);
        args->arg_count = 3;
        is_null[0] = '\0';
        error[0] = '\0';
        args->args = (char **) malloc(sizeof(char *)*3);
        args->args[0] = (char *) malloc(sizeof(char)*(args->lengths[0]));
        args->args[0] = testString1;
        args->args[1] = (char *) malloc(sizeof(char)*(args->lengths[1]));
        args->args[1] = testString2;
        args->args[2] = (char *) limit_arg_ptr;

        my_bool ret = levenshtein_substring_k_init(init, args, message);
        mu_assert("Error, levenshtein_substring_k_test2 => levenshtein_substring_k_init - expected 0", ret == 0);

        longlong result = levenshtein_substring_k(init, args, is_null, error);
        mu_assert("Error, levenshtein_substring_k_test2 => levenshtein_substring_k - expected 2", result == 2);


        levenshtein_substring_k_deinit(init);

        free(args->args);
        free(is_null);
        free(error);
        free(message);
        free(args->arg_type);
        free(args->lengths);
        free(args);
        free(init);
    }

    return 0;
}

static char * levenshtein_k_ratio_test() {
    if(lib_handle != NULL) {
        printf("Testing => %s\n", __FUNCTION__);

        long long limit_arg = 255;
        char *testString1 = "Test String with many white spaces";
        char *testString2 = "This is not a test string with many whitespaces and newlines";


        my_bool (*levenshtein_k_ratio_init)() = dlsym(lib_handle, "levenshtein_k_ratio_init");
        double (*levenshtein_k_ratio)() = dlsym(lib_handle, "levenshtein_k_ratio");
        void(*levenshtein_k_ratio_deinit)() = dlsym(lib_handle, "levenshtein_k_ratio_deinit");

        UDF_INIT *init = (UDF_INIT *) malloc(sizeof(UDF_INIT));
        UDF_ARGS *args = (UDF_ARGS *) malloc(sizeof(UDF_ARGS));
        args->arg_type = (enum Item_result *) malloc(sizeof(enum Item_result)*3);
        args->lengths = (long unsigned int *) malloc(sizeof(long unsigned int)*2);
        char *message = (char *) malloc(sizeof(char)*MYSQL_ERRMSG_SIZE);
        char *error = (char *) malloc(sizeof(char));
        char *is_null = (char *) malloc(sizeof(char));

        long long *limit_arg_ptr = &limit_arg;

        args->arg_type[0] = STRING_RESULT;
        args->arg_type[1] = STRING_RESULT;
        args->arg_type[2] = INT_RESULT;
        args->lengths[0] = strlen(testString1);
        args->lengths[1] = strlen(testString2);
        args->arg_count = 3;
        is_null[0] = '\0';
        error[0] = '\0';
        args->args = (char **) malloc(sizeof(char *)*3);
        args->args[0] = (char *) malloc(sizeof(char)*(args->lengths[0]));
        args->args[0] = testString1;
        args->args[1] = (char *) malloc(sizeof(char)*(args->lengths[1]));
        args->args[1] = testString2;
        args->args[2] = (char *) limit_arg_ptr;

        my_bool ret = levenshtein_k_ratio_init(init, args, message);
        mu_assert("Error, levenshtein_k_ratio_test => levenshtein_k_ratio_init - expected 0", ret == 0);

        double result = levenshtein_k_ratio(init, args, is_null, error);
        mu_assert("Error, levenshtein_k_ratio_test => levenshtein_k_ratio - expected 0.516667", result == 0.5166666666666666);

        levenshtein_k_ratio_deinit(init);

        free(args->args);
        free(is_null);
        free(error);
        free(message);
        free(args->arg_type);
        free(args->lengths);
        free(args);
        free(init);
    }

    return 0;
}

static char * levenshtein_ratio_test() {
    if(lib_handle != NULL) {
        printf("Testing => %s\n", __FUNCTION__);

        char *testString1 = "Test String with many white spaces";
        char *testString2 = "This is not a test string with many whitespaces and newlines";


        my_bool (*levenshtein_ratio_init)() = dlsym(lib_handle, "levenshtein_ratio_init");
        double (*levenshtein_ratio)() = dlsym(lib_handle, "levenshtein_ratio");
        void(*levenshtein_ratio_deinit)() = dlsym(lib_handle, "levenshtein_ratio_deinit");

        UDF_INIT *init = (UDF_INIT *) malloc(sizeof(UDF_INIT));
        UDF_ARGS *args = (UDF_ARGS *) malloc(sizeof(UDF_ARGS));
        args->arg_type = (enum Item_result *) malloc(sizeof(enum Item_result)*2);
        args->lengths = (long unsigned int *) malloc(sizeof(long unsigned int)*2);
        char *message = (char *) malloc(sizeof(char)*MYSQL_ERRMSG_SIZE);
        char *error = (char *) malloc(sizeof(char));
        char *is_null = (char *) malloc(sizeof(char));

        args->arg_type[0] = STRING_RESULT;
        args->arg_type[1] = STRING_RESULT;
        args->lengths[0] = strlen(testString1);
        args->lengths[1] = strlen(testString2);
        args->arg_count = 2;
        is_null[0] = '\0';
        error[0] = '\0';
        args->args = (char **) malloc(sizeof(char *)*2);
        args->args[0] = (char *) malloc(sizeof(char)*(args->lengths[0]));
        args->args[0] = testString1;
        args->args[1] = (char *) malloc(sizeof(char)*(args->lengths[1]));
        args->args[1] = testString2;

        my_bool ret = levenshtein_ratio_init(init, args, message);
        mu_assert("Error, levenshtein_ratio_test => levenshtein_ratio_init - expected 0", ret == 0);

        double result = levenshtein_ratio(init, args, is_null, error);
        mu_assert("Error, levenshtein_ratio_test => levenshtein_ratio - expected 0.516667", result == 0.5166666666666666);

        levenshtein_ratio_deinit(init);

    }

    return 0;
}

static char * damerau_test() {
    if(lib_handle != NULL) {
        printf("Testing => %s\n", __FUNCTION__);

        char *testString1 = "This is a test string";
        char *testString2 = "This is a etst tsring";


        my_bool (*damerau_init)() = dlsym(lib_handle, "damerau_init");
        longlong (*damerau)() = dlsym(lib_handle, "damerau");
        void(*damerau_deinit)() = dlsym(lib_handle, "damerau_deinit");

        UDF_INIT *init = (UDF_INIT *) malloc(sizeof(UDF_INIT));
        UDF_ARGS *args = (UDF_ARGS *) malloc(sizeof(UDF_ARGS));
        args->arg_type = (enum Item_result *) malloc(sizeof(enum Item_result)*2);
        args->lengths = (long unsigned int *) malloc(sizeof(long unsigned int)*2);
        char *message = (char *) malloc(sizeof(char)*MYSQL_ERRMSG_SIZE);
        char *error = (char *) malloc(sizeof(char));
        char *is_null = (char *) malloc(sizeof(char));

        args->arg_type[0] = STRING_RESULT;
        args->arg_type[1] = STRING_RESULT;
        args->lengths[0] = strlen(testString1);
        args->lengths[1] = strlen(testString2);
        args->arg_count = 2;
        is_null[0] = '\0';
        error[0] = '\0';
        args->args = (char **) malloc(sizeof(char *)*2);
        args->args[0] = (char *) malloc(sizeof(char)*(args->lengths[0]));
        args->args[0] = testString1;
        args->args[1] = (char *) malloc(sizeof(char)*(args->lengths[1]));
        args->args[1] = testString2;

        my_bool ret = damerau_init(init, args, message);
        mu_assert("Error, damerau_test => damerau_init - expected 0", ret == 0);

        longlong result = damerau(init, args, is_null, error);
        mu_assert("Error, damerau_test => damerau - expected 2", result == 2);

        damerau_deinit(init);

        free(args->args);
        free(is_null);
        free(error);
        free(message);
        free(args->arg_type);
        free(args->lengths);
        free(args);
        free(init);
    }

    return 0;
}

static char * damerau_substring_test1() {
    if(lib_handle != NULL) {
        printf("Testing => %s\n", __FUNCTION__);

        char *testString1 = "Test String with many white spaces";
        char *testString2 = "This is not a test string with many whitespaces and newlines";


        my_bool (*damerau_substring_init)() = dlsym(lib_handle, "damerau_substring_init");
        longlong (*damerau_substring)() = dlsym(lib_handle, "damerau_substring");
        void(*damerau_substring_deinit)() = dlsym(lib_handle, "damerau_substring_deinit");

        UDF_INIT *init = (UDF_INIT *) malloc(sizeof(UDF_INIT));
        UDF_ARGS *args = (UDF_ARGS *) malloc(sizeof(UDF_ARGS));
        args->arg_type = (enum Item_result *) malloc(sizeof(enum Item_result)*2);
        args->lengths = (long unsigned int *) malloc(sizeof(long unsigned int)*2);
        char *message = (char *) malloc(sizeof(char)*MYSQL_ERRMSG_SIZE);
        char *error = (char *) malloc(sizeof(char));
        char *is_null = (char *) malloc(sizeof(char));

        args->arg_type[0] = STRING_RESULT;
        args->arg_type[1] = STRING_RESULT;
        args->lengths[0] = strlen(testString1);
        args->lengths[1] = strlen(testString2);
        args->arg_count = 2;
        is_null[0] = '\0';
        error[0] = '\0';
        args->args = (char **) malloc(sizeof(char *)*2);
        args->args[0] = (char *) malloc(sizeof(char)*(args->lengths[0]));
        args->args[0] = testString1;
        args->args[1] = (char *) malloc(sizeof(char)*(args->lengths[1]));
        args->args[1] = testString2;

        my_bool ret = damerau_substring_init(init, args, message);
        mu_assert("Error, damerau_substring_test1 => damerau_substring_init - expected 0", ret == 0);

        longlong result = damerau_substring(init, args, is_null, error);
        mu_assert("Error, damerau_substring_test1 => damerau_substring - expected 4", result == 4);

        damerau_substring_deinit(init);

        free(args->args);
        free(is_null);
        free(error);
        free(message);
        free(args->arg_type);
        free(args->lengths);
        free(args);
        free(init);
    }

    return 0;
}

static char * damerau_substring_test2() {
    if(lib_handle != NULL) {
        printf("Testing => %s\n", __FUNCTION__);

        char *testString1 = "Levenhstein";
        char *testString2 = "This is a long string Levenshtein";


        my_bool (*damerau_substring_init)() = dlsym(lib_handle, "damerau_substring_init");
        longlong (*damerau_substring)() = dlsym(lib_handle, "damerau_substring");
        void(*damerau_substring_deinit)() = dlsym(lib_handle, "damerau_substring_deinit");

        UDF_INIT *init = (UDF_INIT *) malloc(sizeof(UDF_INIT));
        UDF_ARGS *args = (UDF_ARGS *) malloc(sizeof(UDF_ARGS));
        args->arg_type = (enum Item_result *) malloc(sizeof(enum Item_result)*2);
        args->lengths = (long unsigned int *) malloc(sizeof(long unsigned int)*2);
        char *message = (char *) malloc(sizeof(char)*MYSQL_ERRMSG_SIZE);
        char *error = (char *) malloc(sizeof(char));
        char *is_null = (char *) malloc(sizeof(char));

        args->arg_type[0] = STRING_RESULT;
        args->arg_type[1] = STRING_RESULT;
        args->lengths[0] = strlen(testString1);
        args->lengths[1] = strlen(testString2);
        args->arg_count = 2;
        is_null[0] = '\0';
        error[0] = '\0';
        args->args = (char **) malloc(sizeof(char *)*2);
        args->args[0] = (char *) malloc(sizeof(char)*(args->lengths[0]));
        args->args[0] = testString1;
        args->args[1] = (char *) malloc(sizeof(char)*(args->lengths[1]));
        args->args[1] = testString2;

        my_bool ret = damerau_substring_init(init, args, message);
        mu_assert("Error, damerau_substring_test2 => damerau_substring_init - expected 0", ret == 0);

        longlong result = damerau_substring(init, args, is_null, error);
        mu_assert("Error, damerau_substring_test2 => damerau_substring - expected 1", result == 1);

        damerau_substring_deinit(init);

        free(args->args);
        free(is_null);
        free(error);
        free(message);
        free(args->arg_type);
        free(args->lengths);
        free(args);
        free(init);
    }

    return 0;
}

static char * all_tests() {
    mu_run_test(strip_w_test_1);
    mu_run_test(strip_w_test_2);
    mu_run_test(strip_w_test_3);
    mu_run_test(levenshtein_k_core_test);
    mu_run_test(levenshtein_test);
    mu_run_test(levenshtein_k_test);
    mu_run_test(levenshtein_substring_k_test1);
    mu_run_test(levenshtein_substring_k_test2);
    mu_run_test(levenshtein_ratio_test);
    mu_run_test(levenshtein_k_ratio_test);
    mu_run_test(damerau_test);
    mu_run_test(damerau_substring_test1);
    mu_run_test(damerau_substring_test2);

    return 0;
}

int main(int argc, char **argv) {
    char *result = 0;
    //lib_handle = dlopen(lib_filename, RTLD_LAZY);
    lib_handle = dlopen(lib_filename, RTLD_NOW|RTLD_GLOBAL);

    if(lib_handle == NULL) {
        printf("Failed loading lib %s\n", lib_filename);
    } else {
        //printf("Loaded lib successfully\n");

        printf("\n");
        result = all_tests();
        if (result != 0) {
            printf("%s\n", result);
        }
        else {
            printf("\nALL TESTS PASSED\n");
        }
        printf("Tests run: %d\n", tests_run);

        dlclose(lib_handle);
    }

    return result != 0;


}