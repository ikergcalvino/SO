CC       = gcc
CFLAGS   = -Wall
LDFLAGS  = -lm
OBJFILES = list.o listproc.o shell.o
TARGET   = shell.out
all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

clean:
	rm -f $(OBJFILES) $(TARGET) *~