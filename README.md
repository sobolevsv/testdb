#### Build on Ubuntu
#### Install cmake

```
sudo apt-get install cmake
```

check GCC version  

```
gcc --version
```


if GCC is not found or version below 9, install GCC 9 

```
apt-get install gcc-9 g++-9
```

Build

```
cd testdb
mkdir build
cd build
cmake ..
make
```

run:

```
./columndb --port 8080
```


#### run in Docker:

```
cd testdb
docker build -t testdb .
docker run -p 8080:8080 --init --rm  testdb:latest
```


#### Test

Download csv file
https://www.kaggle.com/hanselhansel/donorschoose?select=Donors.csv

import file to database

```
curl --location --request POST 'http://localhost:8080/post/donors' \
--header 'Content-Type: text/csv' \
--data-binary '@./Donors.csv'
```

for select

```
 SELECT
  count(*) `donors__count`FROM
  test.donors AS `donors`WHERE
  (`donors`."Donor City" = "San Francisco")
LIMIT
  10000

```

run following curl
  
```
curl --location --request GET 'http://localhost:8080/query?sql=%20SELECT%0A%20%20count(*)%20%60donors__count%60FROM%0A%20%20test.donors%20AS%20%60donors%60WHERE%0A%20%20(%60donors%60.%22Donor%20City%22%20%3D%20%22San%20Francisco%22)%0ALIMIT%0A%20%2010000%0A'
```


for query

```
SELECT
     `donors`."Donor State" `donors__donor_state`,
     count(*) `donors__count`FROM
     test.donors AS `donors`GROUP BY
     1
   ORDER BY
     2 DESC
   LIMIT
     10000
```

run

```
curl --location --request GET 'http://localhost:8080/query?sql=SELECT%0A%20%20`donors`.%22Donor%20State%22%20`donors__donor_state`,%0A%20%20count(*)%20`donors__count`FROM%0A%20%20test.donors%20AS%20`donors`GROUP%20BY%0A%20%201%0AORDER%20BY%0A%20%202%20DESC%0ALIMIT%0A%20%2010000%0A'
```