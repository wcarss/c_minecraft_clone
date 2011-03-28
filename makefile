default: ubuntu

osx: src/ include/
	cd src; make osx;

ubuntu: src/ include/
	cd src; make ubuntu;

clean:
	rm -rf a3
	rm -rf *.o
	cd src; make clean
