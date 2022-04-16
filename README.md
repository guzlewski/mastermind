# mastermind
This is my version of mastermind game. It works on IPC SysV message queue. 

## Compilation
```bash
git clone https://github.com/guzlewski/mastermind
cd mastermind
make
```
If your terminal doesn't support colors: 
``
make nocolor
``

## Usage
```
./server.out [KEY]
then
./client.out [KEY] [NICK]
```

`KEY` - integer, id of the message queue  
`NICK` - string, player name

## Example
```
./server.out 1
./client.out 1 Player
```
![Image](https://sharex.geniush.ovh/a9ed1/biLOKiZE98.png/raw)

## How to play
One player creates puzzle (4 numbers 1-6) and other tries to solve in 12 attempts. 
![Game creating](https://sharex.geniush.ovh/a9ed1/riLinecA54.png/raw)  
If player hit correct number in correct place in result is 2, if correct color in wrong plase result is 1, otherwise 0.
![Playing game](https://sharex.geniush.ovh/a9ed1/dUTOLUtA73.png/raw)  
That means we hit number 2, number 5 is in pattern but on other place, 2 and 6 aren't in pattern.
![Won game](https://sharex.geniush.ovh/a9ed1/ROkAxiGu89.png/raw)
