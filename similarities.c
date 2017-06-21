/*
 * @author Paul Kiman @pkiman
 *         Implemented: substring/fuzzy search with levensthein and damarau-levensthein
 *                      Strip string - trim leading and tailing whitespaces
 *                                  - remove multiple whitespaces and CR/CRLF
 *                      Added Unit test cases
 *                      Added MakeFile
 *
 * Credits to:
*  @author juanmirocks
 * @author Juan Miguel Cejuela, @jmcejuela
 *
 *
 * INSTALLATION
 * Compile Manually
 * (Linux) gcc -v -m64 -Wall -fPIC  -pipe -O3 -o similarities.so -shared similarities.c -I/usr/include/mysql/
 * (mac) gcc -v -bundle -m64 -Wall -fPIC  -pipe -O3 -o similarities.so similarities.c -I/opt/local/include/mysql56/mysql/
 *
 * or run
 *
 * make && make test
 *
 * Compile with alternative include path
 * MYSQL_CFLAGS="-I/opt/local/include/mysql57/mysql/" && make && make test
 *
 * Put the shared library as described in: http://dev.mysql.com/doc/refman/5.0/en/udf-compiling.html
 *
 * Afterwards in SQL:
 *
 * CREATE FUNCTION levenshtein RETURNS INT SONAME 'similarities.so';
 * CREATE FUNCTION levenshtein_k RETURNS INT SONAME 'similarities.so';
 * CREATE FUNCTION levenshtein_ratio RETURNS REAL SONAME 'similarities.so';
 * CREATE FUNCTION levenshtein_k_ratio RETURNS REAL SONAME 'similarities.so';
 * CREATE FUNCTION levenshtein_substring_k RETURNS INT SONAME 'similarities.so';
 * CREATE FUNCTION levenshtein_substring_ci_k RETURNS INT SONAME 'similarities.so';
 * CREATE FUNCTION damerau RETURNS INT SONAME 'similarities.so';
 * CREATE FUNCTION damerau_substring RETURNS INT SONAME 'similarities.so';
 * CREATE FUNCTION damerau_substring_ci RETURNS INT SONAME 'similarities.so';
 *
 * -------------------------------------------------------------------------
 *
 * Some credit for simple levenshtein to: Joshua Drew, SpinWeb Net Designs
 *
 * Other contributors:
 * * popthestack
 * * lilobase
 *
 * -------------------------------------------------------------------------
 *
 * See MySQL UDF documentation pages for details on the implementation of UDF functions.
 *
 */

#include "similarities.h"

#ifdef HAVE_DLOPEN

/* (Expected) maximum number of digits to return */
#define LEVENSHTEIN_MAX 3
#define LENGTH_MAX 255

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

extern char *_strip_w(const char *str, const int str_len);
extern char *_tolowercase(char *str);

/**
 * @param s string
 * @result strip duplicate spaces, CR or CRLF
 */
inline char *_strip_w(const char *str, const int str_len){
    char *striped_str = (char *)malloc((str_len+1));
    enum condition_type {START, INGORE, WRITE};
    enum condition_type condition = START;
    int i,x;
    for (i=x=0; i < str_len; i++){
        if (condition == START && isspace(str[i]))
            continue;
        else if ((isspace(str[i]) && isspace(str[i+1]))
                || (isspace(str[i]) && str[i+1]=='\0')) {
            condition = INGORE;
            continue;
        }
        else if ((!isspace(str[i]) && isspace(str[i+1]))
                || (!isspace(str[i]) && !isspace(str[i+1]))
                || (isspace(str[i]) && !isspace(str[i+1])))
            condition = WRITE;

        if (condition == WRITE)
            striped_str[x++] = str[i];
    }

    striped_str[x] = '\0';
    return striped_str;
}

/**
 * @param s string
 * @result lowercase string
 */
inline char *_tolowercase(char *str){
    int i = 0;
    for(i = 0; str[i]; i++){
        str[i] = tolower(str[i]);
    }
    return str;
}

/**
 * Levenshtein distance
 *
 * @param s string 1 to compare, length n
 * @param t string 2 to compare, length m
 * @result levenshtein distance between s and t
 *
 * @time O(nm), quadratic
 * @space O(nm)
 */
my_bool  levenshtein_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void     levenshtein_deinit(UDF_INIT *initid);
longlong levenshtein(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);


/**
 * Levenshtein distance with threshold k (maximum allowed distance)
 *
 * @param s string 1 to compare, length n
 * @param t string 2 to compare, length m
 * @param k maximum threshold
 * @result levenshtein distance between s and t or >k (not specified) if the distance is greater than k
 *
 * @time O(kl), linear; where l = min(n, m)
 * @space O(k), constant
 */
my_bool  levenshtein_k_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void     levenshtein_k_deinit(UDF_INIT *initid);
longlong levenshtein_k(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);
extern longlong _levenshtein_k_core(const char *s, const int s_len, const char *t, const int t_len, const int k);

/**
 * Levenshtein ratio
 *
 * @param s string 1 to compare, length n
 * @param t string 2 to compare, length m
 * @result levenshtein ratio between s and t
 *
 * @time O(nm), quadratic
 * @space O(nm)
 */
my_bool levenshtein_ratio_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void    levenshtein_ratio_deinit(UDF_INIT *initid);
double  levenshtein_ratio(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);


/**
 * Levenshtein ratio with threshold k (maximum allowed distance)
 *
 * @param s string 1 to compare, length n
 * @param t string 2 to compare, length m
 * @param k maximum threshold
 * @result levenshtein ratio between s and t if (levenshtein distance <= k), otherwise 0.0
 *
 * @time O(kl), linear: where 1 = min(n, m)
 * @space O(k), constant
 */
my_bool levenshtein_k_ratio_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void    levenshtein_k_ratio_deinit(UDF_INIT *initid);
double  levenshtein_k_ratio(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

/**
 * Levenshtein substring distance with threshold k (maximum allowed distance)
 *
 * @param s string 1 to compare, length n
 * @param t string 2 to compare, length m
 * @param k maximum threshold
 * @result levenshtein substring matching distance between s and t or >k (not specified) if the distance is greater than k
 * If the left string is longer than the right string then both strings are swapped
 *
 */
my_bool levenshtein_substring_k_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void    levenshtein_substring_k_deinit(UDF_INIT *initid);
longlong  levenshtein_substring_k(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

/**
 * Levenshtein substring distance with threshold k (maximum allowed distance) case insensitive
 *
 * @param s string 1 to compare, length n
 * @param t string 2 to compare, length m
 * @param k maximum threshold
 * @result levenshtein substring matching distance between s and t or >k (not specified) if the distance is greater than k
 * If the left string is longer than the right string then both strings are swapped
 *
 */
my_bool levenshtein_substring_ci_k_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void    levenshtein_substring_ci_k_deinit(UDF_INIT *initid);
longlong  levenshtein_substring_ci_k(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

/**
 * Damerau-Levenshtein
 *
 * @param s string 1 to compare, length n
 * @param t string 2 to compare, length m
 * @result damerau levenshtein distance between s and t
 *
 */
my_bool damerau_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void damerau_deinit(UDF_INIT *initid);
longlong damerau(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);
extern longlong _damerau_core(const char *str1,int s_len1, const char * str2, int s_len2,
                       const int swap_costs, const int substitute_costs, const int insert_costs, const int delete_costs);

/**
 * Damerau-Levenshtein
 *
 * @param s string 1 to compare, length n
 * @param t string 2 to compare, length m
 * @result damerau levenshtein substring matching distance
 */
my_bool damerau_substring_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void    damerau_substring_deinit(UDF_INIT *initid);
longlong  damerau_substring(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

/**
 * Damerau-Levenshtein case insensitive
 *
 * @param s string 1 to compare, length n
 * @param t string 2 to compare, length m
 * @result damerau levenshtein substring matching distance
 */
my_bool damerau_substring_ci_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void    damerau_substring_ci_deinit(UDF_INIT *initid);
longlong  damerau_substring_ci(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

//-------------------------------------------------------------------------


my_bool levenshtein_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  if ((args->arg_count != 2) ||
      (args->arg_type[0] != STRING_RESULT || args->arg_type[1] != STRING_RESULT)) {
    strcpy(message, "Function requires 2 arguments, (string, string)");
    return 1;
  }

  //matrix for levenshtein calculations of size n+1 x m+1 (+1 for base values)
  int *d = (int *) malloc(sizeof(int) * (args->lengths[0] + 1) * (args->lengths[1] + 1));
  if (d == NULL) {
    strcpy(message, "Failed to allocate memory");
    return 1;
  }

  initid->ptr = (char*) d;
  initid->max_length = LEVENSHTEIN_MAX;
  initid->maybe_null = 0; //doesn't return null

  return 0;
}

void levenshtein_deinit(UDF_INIT *initid) {
  if (initid->ptr != NULL)
    free(initid->ptr);
}

longlong levenshtein(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
  const char *s = args->args[0];
  const char *t = args->args[1];

  int n = (s == NULL) ? 0 : args->lengths[0];
  int m = (t == NULL) ? 0 : args->lengths[1];

  if (0 == n)
    return m;
  if (0 == m)
    return n;

  int *d = (int*) initid->ptr;

  /* Initialization */
  n++; m++;

  int i;
  for (i = 0; i < n; i++)
    d[i] = i;

  int j;
  for (j = 0; j < m; j++)
    d[n * j] = j;

  /* Recurrence */

  int p = 0, h = 0; //indices for d matrix seen as a vector, see below

  int im1 = 0; //i minus 1
  for (i = 1; i < n; i++) {
    p = i;
    int jm1 = 0; //j minus 1
    for (j = 1; j < m; j++) {
      h = p; // d[h] = d[i,j-1], h = (j * n + i - n)  = ((j - 1) * n + i)
      p += n; // d[p] = d[i,j], p = (j * n + i)

      if (s[im1] == t[jm1]) {
        d[p] = d[h-1]; //no operation required, d[i-1,j-1]
      }

      else {
        d[p] = MIN(d[p-1],          //deletion, d[i-1, j]
                   MIN(d[h],        //insertion, d[i, j-1]
                       d[h-1])      //substitution, d[i-1,j-1]
                       ) + 1;       //can put +1 outside because the cost is the same
      }

      jm1 = j;
    }

    im1 = i;
  }

  return (longlong) d[p];
}

//-------------------------------------------------------------------------

my_bool levenshtein_ratio_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  if ((args->arg_count != 2) ||
      (args->arg_type[0] != STRING_RESULT || args->arg_type[1] != STRING_RESULT)) {
    strcpy(message, "Function requires 2 arguments, (string, string)");
    return 1;
  }

  //matrix for levenshtein calculations of size n+1 x m+1 (+1 for base values)
  int *d = (int *) malloc(sizeof(int) * (args->lengths[0] + 1) * (args->lengths[1] + 1));
  if (d == NULL) {
    strcpy(message, "Failed to allocate memory");
    return 1;
  }

  initid->ptr = (char*) d;
  initid->max_length = LEVENSHTEIN_MAX;
  initid->maybe_null = 0; //doesn't return null

  return 0;
}

void levenshtein_ratio_deinit(UDF_INIT *initid) {
  if (initid->ptr != NULL)
    free(initid->ptr);
}

double levenshtein_ratio(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
  const char *s = args->args[0];
  const char *t = args->args[1];

  int n = (s == NULL) ? 0 : args->lengths[0];
  int m = (t == NULL) ? 0 : args->lengths[1];
  double maxlen = MAX(n, m);
  if (maxlen == 0)
    return 0.0;

  double dist = (double) levenshtein(initid, args, is_null, error);
  return 1.0 - dist/maxlen;
}

//-------------------------------------------------------------------------

my_bool levenshtein_k_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  if ((args->arg_count != 3) ||
      (args->arg_type[0] != STRING_RESULT || args->arg_type[1] != STRING_RESULT || args->arg_type[2] != INT_RESULT)) {
    strcpy(message, "Function requires 3 arguments, (string, string, int)");
    return 1;
  }

  initid->ptr = NULL;
  initid->max_length = LEVENSHTEIN_MAX;
  initid->maybe_null = 0; //doesn't return null

  return 0;
}

/**
 *  deallocate memory, clean and close
 */
void levenshtein_k_deinit(UDF_INIT *initid) {
    if (initid->ptr != NULL)
        free(initid->ptr);
}

/*
 * 1st observation: time O(kl)
 * (see Algorithms on Strings, Trees, and Sequences, Dan Gusfield, pg. 263)
 *
 * When the levenshtein distance is limited to k the alignment path cannot be >k
 * cells off the main diagonal, either from the left (insertions) or from the
 * right (deletions) This means we don't have to fill the whole recurrence
 * matrix; it suffices to fill a strip of 2k + 1 cells in a row. Also, r <= k,
 * for r = m - n, is a necessary condition for there to be any solution.
 *
 *
 * 2nd observation: exactly k + 1
 *
 * The number of _unpaired_ insertions/deletions (the absolute difference in the
 * number of these operations) is as maximum k. When n == m (r == 0) any
 * movement to the left, insertion, in the recurrence matrix is corresponded to
 * a movement to the right, deletion, or viceversa. Analogously when m > n only
 * a maximum of k insertions/deletions operations can be left unpaired. This
 * makes the required strip size to be k + 1 cells.
 *
 * Call l = min(n, m). Place the string with length l on the y axis, and the
 * other on the x axis. By doing this, the number of rows is equal or less than
 * the number of columns and so the algorithm effectively runs in O(kl) Also, by
 * this arrangement of s and t in the recurrence matrix, r indicates the forced
 * number of unpaired deletions, i.e., since r>=0, there cannot be unpaired
 * deletions. Observe that for there to be any insertion k must be equal or
 * greater than 2.
 *
 * This arrangement leaves (k-r)/2 possible insertions (left) and (k-r)/2 + r
 * possible deletions (right) --> (k-r)/2 + (k-r)/2 + r + 1 == k + 1
 *
 *
 * 3rd observation: space O(k)
 *
 * As with the original levenshtein algorithm, only the current and last rows
 * are needed (that is, when we don't do traceback because we need not the
 * alignment path). By the previous observation, we just need 2 rows of k + 1
 * cells. For this, considering the last row, the previous diagonal is actually
 * in the same column (substitution) and the above cell is actually in the next
 * column (deletion). Observe that by doing this, we're virtually creating a
 * recurrence matrix of n * (k+1) being now the original diagonal in the middle
 * column (-r) (matrix which could be used to do the traceback)
 *
 */
longlong levenshtein_k(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
  char *s = args->args[0];
  char *t = args->args[1];
  const int k = *((int*) args->args[2]);

  return _levenshtein_k_core(s,args->lengths[0], t, args->lengths[1], k);
}

inline longlong _levenshtein_k_core(const char *s, const int s_len, const char *t, const int t_len, const int k) {

  int n = (s == NULL) ? 0 : s_len;
  int m = (t == NULL) ? 0 : t_len;

  //order the strings so that the first always has the minimum length l
  if (n > m) {
    int aux = n;
    n = m;
    m = aux;
    const char *auxs = s;
    s = t;
    t = auxs;
  }

  const int ignore = k + 1; //lev dist between s and t is at least greater than k
  const int r = m - n;

  if (0 == n)
    return (m > k) ? ignore : m;
  if (0 == m)
    return (n > k) ? ignore : n;
  if (r > k)
    return ignore;

  const int lsize = (((k > m) ? m : k) - r) / 2; //left space for insertions
  const int rsize = lsize + r; //right space for deletions, rsize >= lsize (rsize == lsize iff r == 0)
  const int stripsize = lsize + rsize + 1; // + 1 for the diagonal cell
  const int stripsizem1 = stripsize - 1; //see later, not to repeat calculations

  int d[2 * stripsize]; //Current and last rows
  int currentrow;
  int lastrow;

  /* Initialization */

  //currentrow = 0
  int i;
  for (i = lsize; i < stripsize; i++) //start from diagonal cell
    d[i] = i - lsize;

  /* Recurrence */

  currentrow = stripsize;
  lastrow = 0;

  //j index for virtual recurrence matrix, jv index for rows
  //bl & br = left & right bounds for j
  int j, jv, bl, br;
  int im1 = 0, jm1;
  int a, b, c, min; //for minimum function, coded directly here for maximum speed
  for (i = 1; i <= n; i++) {

    //bl = max(i - lsize, 0), br = min(i + rsize, m)
    bl = i - lsize;
    if (bl < 0) {
      jv = abs(bl); //no space for all allowed insertions
      bl = 0;
    }
    else
      jv = 0;
    br = i + rsize;
    if (br > m)
      br = m;

    jm1 = bl - 1;
    for (j = bl; j <= br; j++) {
      if (0 == j) //postponed part of initialization
        d[currentrow + jv] = i;
      else {
        //By observation 3, the indices change for the lastrow (always +1)
        if (s[im1] == t[jm1]) {
          d[currentrow + jv] = d[lastrow + jv];
        }
        else {
          //get the minimum of these 3 operations
          a = (0 == jv) ? ignore : d[currentrow + jv - 1]; //deletion
          b = (stripsizem1 == jv) ? ignore : d[lastrow + jv + 1]; //insertion
          c = d[lastrow + jv]; //substitution

          min = a;
          if (b < min)
            min = b;
          if (c < min)
            min = c;

          d[currentrow + jv] = min + 1;
        }
      }
      jv++;
      jm1 = j;
    }

    //obsv: the cost of a following diagonal never decreases
    if (d[currentrow + lsize + r] > k)
      return ignore;

    im1 = i;

    //swap
    currentrow = currentrow ^ stripsize;
    lastrow = lastrow ^ stripsize;
  }

  //only here if levenhstein(s, t) <= k
  return (longlong) d[lastrow + lsize + r]; //d[n, m]
}

//-------------------------------------------------------------------------

my_bool levenshtein_k_ratio_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  if ((args->arg_count != 3) ||
      (args->arg_type[0] != STRING_RESULT || args->arg_type[1] != STRING_RESULT || args->arg_type[2] != INT_RESULT)) {
    strcpy(message, "Function requires 3 arguments, (string, string, int)");
    return 1;
  }

  initid->ptr = NULL;
  initid->max_length = LEVENSHTEIN_MAX;
  initid->maybe_null = 0; //doesn't return null

  return 0;
}

void levenshtein_k_ratio_deinit(UDF_INIT *initid) {
    if (initid->ptr != NULL)
        free(initid->ptr);
}

double levenshtein_k_ratio(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
  const char *s = args->args[0];
  const char *t = args->args[1];
  const int k = *((int*) args->args[2]);

  int n = (s == NULL) ? 0 : args->lengths[0];
  int m = (t == NULL) ? 0 : args->lengths[1];
  double maxlen = MAX(n, m);
  if (maxlen == 0)
    return 0.0;

  double dist = (double)levenshtein_k(initid, args, is_null, error);
  if (dist > k)
    return 0.0;
  else
    return 1.0 - dist/maxlen;
}

//-------------------------------------------------------------------------

my_bool levenshtein_substring_k_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  // sanitizing input parameters
  if ((args->arg_count != 3) ||
      (args->arg_type[0] != STRING_RESULT || args->arg_type[1] != STRING_RESULT || args->arg_type[2] != INT_RESULT)) {
    strcpy(message, "Function requires 3 arguments, (string, string, int)");
    return 1;
  }

  initid->ptr = NULL;
  initid->max_length = LEVENSHTEIN_MAX;
  initid->maybe_null = 0; //doesn't return null

  return 0;
}

/**
 *  deallocate memory, clean and close
 */
void levenshtein_substring_k_deinit(UDF_INIT *initid) {
    if (initid->ptr != NULL)
        free(initid->ptr);
}

longlong levenshtein_substring_k(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
  const char *s = args->args[0];
  const char *t = args->args[1];

  const int k = *((int*) args->args[2]);

  int n = (s == NULL) ? 0 : args->lengths[0]; /*contains pattern*/
  int m = (t == NULL) ? 0 : args->lengths[1]; /*contains row*/

  const char *s_stripped = _strip_w(s, n);
  const char *t_stripped = _strip_w(t, m);

  n = strlen(s_stripped);
  m = strlen(t_stripped);

  //order the strings so that the first always has the minimum length l
  if (n > m) {
    int aux = n;
    n = m;
    m = aux;
    const char *auxs = s_stripped;
    s_stripped = t_stripped;
    t_stripped = auxs;
  }

  longlong lowest_dist = LLONG_MAX;
  longlong dist = 0;

  unsigned int index = 0;

  while (index <= (m - n)) {
    dist = _levenshtein_k_core(s_stripped, n, t_stripped, n, k);
    if (dist < lowest_dist)
        lowest_dist = dist;

    (void)*t_stripped++;
    index++;
    if (lowest_dist == 1)
        return lowest_dist;
  }

  return lowest_dist;
}

//-------------------------------------------------------------------------

my_bool levenshtein_substring_ci_k_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  // sanitizing input parameters
  if ((args->arg_count != 3) ||
      (args->arg_type[0] != STRING_RESULT || args->arg_type[1] != STRING_RESULT || args->arg_type[2] != INT_RESULT)) {
    strcpy(message, "Function requires 3 arguments, (string, string, int)");
    return 1;
  }

  initid->ptr = NULL;
  initid->max_length = LEVENSHTEIN_MAX;
  initid->maybe_null = 0; //doesn't return null

  return 0;
}

/**
 *  deallocate memory, clean and close
 */
void levenshtein_substring_ci_k_deinit(UDF_INIT *initid) {
    if (initid->ptr != NULL)
        free(initid->ptr);
}

longlong levenshtein_substring_ci_k(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
  const char *s = args->args[0];
  const char *t = args->args[1];

  const int k = *((int*) args->args[2]);

  int n = (s == NULL) ? 0 : args->lengths[0]; /*contains pattern*/
  int m = (t == NULL) ? 0 : args->lengths[1]; /*contains row*/

  const char *s_stripped = _tolowercase(_strip_w(s, n));
  const char *t_stripped = _tolowercase(_strip_w(t, m));

  n = strlen(s_stripped);
  m = strlen(t_stripped);

  //order the strings so that the first always has the minimum length l
  if (n > m) {
    int aux = n;
    n = m;
    m = aux;
    const char *auxs = s_stripped;
    s_stripped = t_stripped;
    t_stripped = auxs;
  }

  longlong lowest_dist = LLONG_MAX;
  longlong dist = 0;

  unsigned int index = 0;

  while (index <= (m - n)) {
    dist = _levenshtein_k_core(s_stripped, n, t_stripped, n, k);
    if (dist < lowest_dist)
        lowest_dist = dist;

    (void)*t_stripped++;
    index++;
    if (lowest_dist == 1)
        return lowest_dist;
  }

  return lowest_dist;
}

//-------------------------------------------------------------------------

//! check parameters and allocate memory for MySql
my_bool damerau_init(UDF_INIT *init, UDF_ARGS *args, char *message) {

    // make sure user has provided three arguments
    if (args->arg_count != 2) {
        strncpy(message, "DAMERAU() requires three arguments", 80);
        return 1;
    } else {
        // sanitize parameters
        if ( args->arg_type[0] != STRING_RESULT ||
            args->arg_type[1] != STRING_RESULT) {
            strncpy(message, "DAMERAU() requires arguments (string, string", 80);
            return 1;
        }
    }

    return 0;
}

//! check parameters akkd allocate memory for MySql
longlong damerau(UDF_INIT *init, UDF_ARGS *args, char *is_null, char *error) {

    // s is the first user-supplied argument; t is the second
    const char *str1 = args->args[0];
    const char *str2 = args->args[1];

    const int len1 = (str1 == NULL) ? 0 : args->lengths[0];
    const int len2 = (str2 == NULL) ? 0 : args->lengths[1];

    return _damerau_core(
         str1, len1,
         str2, len2,
        /* swap */              1,
        /* substitution */	    1,
        /* insertion */         1,
        /* deletion */          1
    );
}

/**
 *  deallocate memory, clean and close
 */
void damerau_deinit(UDF_INIT *initid) {
    if (initid->ptr != NULL)
        free(initid->ptr);
}

/**
 * core levenshtein damerau_core function
 * @param string str1 to compare
 * @param int length1
 * @param string str2 to compare, length s_len2
 * @param int length2
 * @param int costs to swap
 * @param int costs to substitute
 * @param int costs to insert
 * @param int costs to delete
 */
inline longlong _damerau_core(const char *str1,int s_len1,
                       const char * str2, int s_len2,
                       const int swap_costs, const int substitute_costs, const int insert_costs, const int delete_costs) {

    int d[s_len1+1][s_len2+1];

    int i, j, l_cost;

    for (i = 0;i <= s_len1;i++) {
        d[i][0] = i;
    }
    for(j = 0; j<= s_len2; j++) {
        d[0][j] = j;
    }
    for (i = 1;i <= s_len1;i++) {
        for(j = 1; j<= s_len2; j++) {
            if( str1[i-1] == str2[j-1] )
                l_cost = 0;
            else
                l_cost = 1;

            d[i][j] = MIN(
                d[i-1][j] + delete_costs,                   // delete
                MIN(d[i][j-1] + insert_costs,               // insert
                    d[i-1][j-1] + l_cost*substitute_costs)  // substitution
            );
            if( (i > 1) && (j > 1) &&
                (str1[i-1] == str2[j-2]) && (str1[i-2] == str2[j-1])) {

                d[i][j] = MIN(
                    d[i][j],
                    d[i-2][j-2] + l_cost*swap_costs         // swap
                );
            }
        }
    }
    return d[s_len1][s_len2];
}

//-------------------------------------------------------------------------

my_bool damerau_substring_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  // sanitizing input parameters
  if ((args->arg_count != 2) ||
      (args->arg_type[0] != STRING_RESULT || args->arg_type[1] != STRING_RESULT)) {
    strcpy(message, "Function requires 2 arguments, (string, string)");
    return 1;
  }

  initid->ptr = NULL;
  initid->max_length = LEVENSHTEIN_MAX;
  initid->maybe_null = 0; //doesn't return null

  return 0;
}

/**
 *  deallocate memory, clean and close
 */
void damerau_substring_deinit(UDF_INIT *initid) {
    if (initid->ptr != NULL)
        free(initid->ptr);
}

longlong damerau_substring(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
  const char *s = args->args[0];
  const char *t = args->args[1];

  int n = (s == NULL) ? 0 : args->lengths[0]; /*contains pattern*/
  int m = (t == NULL) ? 0 : args->lengths[1]; /*contains row*/

  const char *s_stripped = _strip_w(s, n);
  const char *t_stripped = _strip_w(t, m);

  n = strlen(s_stripped);
  m = strlen(t_stripped);

  //order the strings so that the first always has the minimum length l
  if (n > m) {
    int aux = n;
    n = m;
    m = aux;
    const char *auxs = s_stripped;
    s_stripped = t_stripped;
    t_stripped = auxs;
  }

  longlong lowest_dist = LLONG_MAX;
  longlong dist = 0;

  unsigned int index = 0;

  while (index <= (m - n)) {
    dist = _damerau_core(
                s_stripped, n,
                t_stripped, n,
                /* swap */              1,
                /* substitution */	    1,
                /* insertion */         1,
                /* deletion */          1
    );
    if (dist < lowest_dist)
        lowest_dist = dist;

    (void)*t_stripped++;
    index++;
    if (lowest_dist == 1)
        return lowest_dist;
  }

  return lowest_dist;
}

//-------------------------------------------------------------------------

my_bool damerau_substring_ci_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
  // sanitizing input parameters
  if ((args->arg_count != 2) ||
      (args->arg_type[0] != STRING_RESULT || args->arg_type[1] != STRING_RESULT)) {
    strcpy(message, "Function requires 2 arguments, (string, string)");
    return 1;
  }

  initid->max_length = LEVENSHTEIN_MAX;
  initid->maybe_null = 0; //doesn't return null

  return 0;
}

/**
 *  deallocate memory, clean and close
 */
void damerau_substring_ci_deinit(UDF_INIT *initid) {
    if (initid->ptr != NULL)
        free(initid->ptr);
}

longlong damerau_substring_ci(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
  const char *s = args->args[0];
  const char *t = args->args[1];

  int n = (s == NULL) ? 0 : args->lengths[0]; /*contains pattern*/
  int m = (t == NULL) ? 0 : args->lengths[1]; /*contains row*/

  const char *s_stripped = _tolowercase(_strip_w(s, n));
  const char *t_stripped = _tolowercase(_strip_w(t, m));

  n = strlen(s_stripped);
  m = strlen(t_stripped);

  //order the strings so that the first always has the minimum length l
  if (n > m) {
    int aux = n;
    n = m;
    m = aux;
    const char *auxs = s_stripped;
    s_stripped = t_stripped;
    t_stripped = auxs;
  }

  longlong lowest_dist = LLONG_MAX;
  longlong dist = 0;

  unsigned int index = 0;

  while (index <= (m - n)) {
    dist = _damerau_core(
                s_stripped, n,
                t_stripped, n,
                /* swap */              1,
                /* substitution */	    1,
                /* insertion */         1,
                /* deletion */          1
    );
    if (dist < lowest_dist)
        lowest_dist = dist;

    (void)*t_stripped++;
    index++;
    if (lowest_dist == 1)
        return lowest_dist;
  }

  return lowest_dist;
}

#endif /* HAVE_DLOPEN */
