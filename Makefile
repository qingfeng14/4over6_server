
OBJS = main.o error_check.o
HEADS = unp.h

main: $(OBJS) $(HEAD)
	g++ -o $@ $(OBJS)

%.o: %.cpp
	g++ -c $< -o $@

clean: 
	rm main *.o


