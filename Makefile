CXXLAGS=-Os `pkg-config --cflags --libs libevdev`

default: push-to-talk

push-to-talk: push-to-talk.cpp
	gcc push-to-talk.cpp $(CXXLAGS) -o push-to-talk

clean:
	rm -f push-to-talk