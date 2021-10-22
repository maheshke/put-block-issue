# Reproduce PutSnapshotBlock issue with AWS CPP SDK

## Build program
```sh
$ make
g++ -W -Wall -Werror -std=c++11  -c snapshot.cpp -o snapshot.o
g++ -W -Wall -Werror -std=c++11 snapshot.o -o put-block-issue -laws-cpp-sdk-ebs -laws-cpp-sdk-core
```

## Create an empty snapshot
```sh
$ aws ebs start-snapshot --volume-size 1 --tags Key=test,Value=test
```

## Reproduce issue
```sh
./put-block-issue --snapshot <Snapshot ID>
```

* Example output:

```sh
$ ./put-block-issue --snapshot "snap-08fd7d63ffe3ecf8a"
Failed to put block. Snapshot snap-042b76f2513c1ab98. Exception - : curlCode: 56, Failure when receiving data from the peer
```
