**MySQL UDF functions implemented in C for**

* General Levenshtein algorithm
* k-bounded Levenshtein distance algorithm (linear time, constant space),
* Levenshtein ratio (syntactic sugar for: `levenshtein_ratio(s, t) = 1 - levenshtein(s, t) / max(s.length, t.length)`)
* k-bounded Levenshtein ratio
* Fuzzy search with levensthein
* Fuzzy search with damerau-levensthein
* native unit testing

**How to compile?**

`make && make test`

Compile with another location of mysql include folder use:
`MYSQL_CFLAGS="-I/opt/local/include/mysql57/mysql/" && make && make test`

**How to install?**

Find out the plugin_dir of your MySQL server:

```
mysql> SHOW GLOBAL VARIABLES LIKE 'plugin_dir';
+---------------+--------------------------------+
| Variable_name | Value                          |
+---------------+--------------------------------+
| plugin_dir    | /opt/local/lib/mysql56/plugin/ |
+---------------+--------------------------------+
1 row in set (0.00 sec)
```

Copy the compiled similarities.so (linux/mac) or similarities.dll (windows) to the plugin_dir

Register the udf functions in MySQL which you need:
```
CREATE FUNCTION levenshtein RETURNS INT SONAME 'similarities.so';
CREATE FUNCTION levenshtein_k RETURNS INT SONAME 'similarities.so';
CREATE FUNCTION levenshtein_ratio RETURNS REAL SONAME 'similarities.so';
CREATE FUNCTION levenshtein_k_ratio RETURNS REAL SONAME 'similarities.so';
CREATE FUNCTION levenshtein_substring_k RETURNS INT SONAME 'similarities.so';
CREATE FUNCTION damerau RETURNS INT SONAME 'similarities.so';
CREATE FUNCTION damerau_substring RETURNS INT SONAME 'similarities.so';
```

**How to uninstall?**

```
DROP FUNCTION levenshtein;
DROP FUNCTION levenshtein_k';
DROP FUNCTION levenshtein_ratio;
DROP FUNCTION levenshtein_k_ratio;
DROP FUNCTION levenshtein_substring_k;
DROP FUNCTION damerau;
DROP FUNCTION damerau_substring; 
```

**How to use?**

*Levenshtein Distance*
```
mysql> SELECT LEVENSHTEIN("Levenhstein", "Levenshtein") AS distance;
+----------+
| distance |
+----------+
|        2 |
+----------+
1 row in set (0.00 sec)
```

*Levenshtein-Damerau Distance*
```
mysql> SELECT DAMERAU("Levenhstein", "Levenshtein") AS distance;
+----------+
| distance |
+----------+
|        1 |
+----------+
1 row in set (0.00 sec)
```

*Levenshtein Fuzzy-Search Distance*
```
mysql> SELECT LEVENSHTEIN_SUBSTRING_K("Levenhstein", "This is a long string Levenshtein", 255) AS distance;
+----------+
| distance |
+----------+
|        2 |
+----------+
1 row in set (0.00 sec)
```

*Levenshtein-Damerau Fuzzy-Search Distance*
```
mysql> SELECT DAMERAU_SUBSTRING("Levenhstein", "This is a long string Levenshtein") AS distance;
+----------+
| distance |
+----------+
|        1 |
+----------+
1 row in set (0.00 sec)
```
