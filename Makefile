output: serverM serverC serverEE serverCS client

all: serverM serverC serverEE serverCS client

serverM:
	g++ -std=c++11 -shared-libgcc serverM.cpp -o serverM

serverC:
	g++ -std=c++11  -shared-libgcc serverC.cpp -o serverC

serverEE:
	g++ -std=c++11  -shared-libgcc serverEE.cpp -o serverEE

serverCS:
	g++ -std=c++11  -shared-libgcc serverCS.cpp -o serverCS

client:
	g++ -std=c++11  -shared-libgcc client.cpp -o client

clean:
	rm -rf serverM serverC serverEE serverCS client