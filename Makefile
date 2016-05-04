TARGET = serialecho

$(TARGET) : serialecho.c 
	$(CC) $(CFLAGS) serialecho.c -o $(TARGET)

clean:
	rm -f $(TARGET)

