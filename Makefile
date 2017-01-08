TARGET = serialecho

$(TARGET) : serialecho.c 
	$(CC) serialecho.c -o $(TARGET)

clean:
	rm -f $(TARGET)
