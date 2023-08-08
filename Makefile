CC         = gcc
CFLAGS     = -g
LDFLAGS    = 
OBJFILES   = scheduler.o main.o
TARGET     = scheduler

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

clean:
	rm -f $(OBJFILES) $(TARGET) *~
