CC=gcc
CFLAGS = -Icspice/include
LDFLAGS = -lglut -lGLU -lGL
LDFLAGS += cspice/lib/cspice.a
LDFLAGS += -lm

APP = solar
DEPS = solar.c parse_hyg.c
OBJ = $(patsubst %.c,%.o,$(DEPS))

all: $(APP)

hygdata_v3.csv: 
	gunzip -f hygdata_v3.csv.gz

cspice/lib/cspice.a:
	gunzip -f cspice.tar.Z
	cd cspice && ./makeall.csh

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(APP): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(OBJ) $(PARSER).o

.PHONY: clean
