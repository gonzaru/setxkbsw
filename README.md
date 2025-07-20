# setxkbsw - set the X keyboard switch

### Installation

#### 1. Clone

    $ git clone https://github.com/gonzaru/setxkbsw.git
    $ cd setxkbsw

#### 2. Build

    $ mkdir -p bin
    $ sh build.sh

#### 3. Copy

    # copy the file to any searchable shell $PATH, for example:
    $ sudo cp bin/setxkbsw /usr/local/bin/

#### Usage of setxkbsw:

* shows help

```
$ setxkbsw -h
```

#### Examples:

* switch to layout group by index [0..3]

```
$ setxkbsw -s N
```

* switch to the first layout group by index [0..3]

```
$ setxkbsw -s 0
```

* switch to the Russian layout group (by name)

```
$ setxkbsw -s ru
```

* switch to the next layout group (toggle) 

```
$ setxkbsw -n
```

* prints the current layout group (short version)

```
$ setxkbsw -p
```

* prints the current layout group (long version) 

```
$ setxkbsw -P
```

* prints all layout groups (short version) 

```
$ setxkbsw -l
```

* prints all layout groups (long version)

```
$ setxkbsw -L
```



