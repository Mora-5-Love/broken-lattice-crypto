FLAGS = -DDEBUG -DDEBUG1 -DAPPLY_LLL -DDIAG_DOMINANT
ETC_FLAGS = -DDEBUG2 -DBABAI_ROUNDING 

all: ggh-gen-keys ggh-encrypt ggh-decrypt

ggh-gen-keys: ggh-gen-keys.cpp
	g++ $(FLAGS) -I/usr/local/include -L/usr/local/lib ggh-gen-keys.cpp -o ggh-gen-keys -lntl -lm

ggh-encrypt: ggh-encrypt.cpp
	g++ $(FLAGS) -I/usr/local/include -L/usr/local/lib ggh-encrypt.cpp -o ggh-encrypt -lntl -lm

ggh-decrypt: ggh-decrypt.cpp
	g++ $(FLAGS) -I/usr/local/include -L/usr/local/lib ggh-decrypt.cpp -o ggh-decrypt -lntl -lm

clean:
	rm -f ggh-gen-keys ggh-encrypt ggh-decrypt

