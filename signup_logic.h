#ifndef SIGNUP_LOGIC_H
#define SIGNUP_LOGIC_H

// Ensure wchar_t is declared before using it in prototypes
#include <wchar.h>

/*
 * Attempts to create a new user.  The user ID must be unique in the
 * user.txt file.  Returns 1 on success and 0 on failure (e.g. empty
 * fields or duplicate ID).  The user.txt file is encoded in
 * UTF-16LE and stores one user per line in the format:
 *     ID PW NAME
 * separated by whitespace.  This function appends a new record to
 * that file when the ID does not already exist.
 */
int Signup_CreateUser(const wchar_t* id, const wchar_t* pw, const wchar_t* name);

#endif // SIGNUP_LOGIC_H