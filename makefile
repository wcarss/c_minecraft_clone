default: ubuntu

osx: src/ include/
	cd src; make osx;

ubuntu: src/ include/
	cd src; make ubuntu;

new: src/ include/
	cd src; make new;

clean:
	rm -rf a3
	rm -rf new_mc
	rm -rf *.o
	cd src; make clean
