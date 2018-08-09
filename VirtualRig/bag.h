#ifndef BAG_H
#define BAG_H

#include <assert.h>
#include "utils.h"
#include "math.h"

template <class ITERATOR_ELEMENT> class Iterator;

enum BAG_ELEMENT_MARK { BAG_MARK_NULL, BAG_MARK_DELETE, BAG_MARK_PRESENT };

#define MAX_ITERATORS 1

#define LARGE_PRIME_A 10007
#define LARGE_PRIME_B 11003
#define LARGE_PRIME_C 12007

int NextLargestPrime(unsigned int x);  // defined in utils.C
// ======================================================================
//*! 
// \brief:
// A bag is implemented with a hash table to allow efficient access and removal
// coded by Leonardo Fischer
//======================================================================
template <class BAG_ELEMENT>
class Bag {

public:

	// ========================
	// CONSTRUCTOR & DESTRUCTOR
	Bag(int s, void (*e_func)(BAG_ELEMENT, int &a, int &b, int &c)) {
		extract_func = e_func;
		num_iterators = 0;
		size = NextLargestPrime(s);
		marks = new enum BAG_ELEMENT_MARK[size];
		data = new BAG_ELEMENT[size];
		for (int i = 0; i < size; i++)
			marks[i] = BAG_MARK_NULL;
		count = 0;
		del_count = 0; }
	virtual ~Bag() {
		assert (num_iterators == 0);
		delete [] data;
		delete [] marks; }

	// =========
	// ACCESSORS
	int Count() const { return count; }
	int Member(const BAG_ELEMENT e) const {
		int a,b,c;
		assert (e != (BAG_ELEMENT)0);
		extract_func(e,a,b,c);
		// lets put c=0 in order to reduce reduncey
		//c=0;
  		int orig = hash(a,b,c);
		int x = orig;
		//cout << " \n \n " << a << " "  << b << " "  << " " << c << "\n";
		while (1) {
			if (marks[x] == BAG_MARK_NULL)
				return 0;
			if (marks[x] == BAG_MARK_PRESENT && data[x] == e)
				return 1;
			x = skip(orig,x);
		}
	}

	BAG_ELEMENT ChooseRandom() const {
		assert (Count() > 0);
		while (1) {    
			int random_int = int(floor(drand48()*size));
			if (marks[random_int] == BAG_MARK_PRESENT)
				return data[random_int];
		}
	}

	BAG_ELEMENT GetReorder(int a, int b, int c) const {
		assert (a != b && b != c && a != c);
		if (a < b && a < c) 
			return Get(a,b,c);
		else if (b < a && b < c)
			return Get(b,c,a);
		assert (c < a && c < b);
		return Get(c,a,b); }


	BAG_ELEMENT GetReorder(int a, int b) const {
		assert (a != b);
		if (a < b) return Get(a,b);
		return Get(b,a); }

	BAG_ELEMENT Get(int a, int b) const {
		assert (a != b);
		int orig = hash(a,b,0);
		int x = orig;
		while (1) {
			assert (x >= 0 && x < size);
			if (marks[x] == BAG_MARK_NULL)
				return 0;
			if (marks[x] == BAG_MARK_PRESENT) {
				int _a,_b,_c;
				assert (data[x] != (BAG_ELEMENT)0);
				extract_func(data[x],_a,_b,_c);
				if (_a ==a && _b == b) {
					return data[x];
				}
			}
			x = skip(orig,x);
		}
	}

	Iterator<BAG_ELEMENT>* StartIteration() {
		//printf ("start iteration %d   %d\n", num_iterators, (int)this);
		assert (num_iterators < MAX_ITERATORS);
		num_iterators++;
		return new Iterator<BAG_ELEMENT>(this);
	}

	void EndIteration(Iterator<BAG_ELEMENT> *&iter) {
		assert (num_iterators > 0);
		num_iterators--;
		assert (iter != NULL);
		delete iter;
		iter = NULL;    
	}

	// =========
	// MODIFIERS
	void Add(const BAG_ELEMENT e) {
		assert(!Member(e));
		if (count + del_count > size / 2)
			Resize(max2(count*4,size));
		int a,b,c;
		assert (e != (BAG_ELEMENT)0);
		extract_func(e,a,b,c);
		int orig = hash(a,b,c);
		int x = orig;
		while (marks[x] == BAG_MARK_PRESENT)
			x = skip(orig,x);
		if (marks[x] == BAG_MARK_DELETE)
			del_count--;
		marks[x] = BAG_MARK_PRESENT;
		data[x] = e;
		count++;
	}

	void AddNoDuplicates(const BAG_ELEMENT e) { if (!Member(e)) Add(e); }

	void Remove(const BAG_ELEMENT e) {
		int a,b,c;
		assert (e != (BAG_ELEMENT)0);
		extract_func(e,a,b,c);
		int orig = hash(a,b,c);
		int x = orig;
		while (1) {
			assert (marks[x] != BAG_MARK_NULL);
			if (marks[x] == BAG_MARK_PRESENT && data[x] == e) {
				marks[x] = BAG_MARK_DELETE;
				del_count++;
				count--;
				break;
			}
			x = skip(orig,x);
		}
	}

	void DeleteAllElements() {
		assert(num_iterators == 0);
		for (int i = 0; i < size; i++) {
			if (marks[i] == BAG_MARK_PRESENT)
				delete data[i];
			marks[i] = BAG_MARK_NULL;
		}
		del_count = 0;
		count = 0;
	}

	void Clear() {
		assert(num_iterators == 0);
		for (int i = 0; i < size; i++) {
			marks[i] = BAG_MARK_NULL;
		} 
		del_count = 0;
		count = 0;
	}

	void Print() {
		printf ("BAG::PRINT %d %d %d\n",size,count,del_count);
		int c=0;
		for (int i = 0; i < size; i++) {      
			printf ("%3d: ",i);
			if (marks[i] == BAG_MARK_PRESENT) {
				data[i]->Print();
				c++;
			} else if (marks[i] == BAG_MARK_DELETE) {
				printf ("XXXXXXXXXX\n");
			} else {
				assert (marks[i] == BAG_MARK_NULL);
				printf ("NULL\n");
			}
		}
		assert(c==count);
	}

private:
	void Resize(int s) {
		assert (s > 0 && s > count);
		// save old stuff
		int old_size = size;
		BAG_ELEMENT *old_data = data;
		enum BAG_ELEMENT_MARK *old_marks = marks;
		// make new space
		size = NextLargestPrime(s);
		marks = new enum BAG_ELEMENT_MARK[size];
		data = new BAG_ELEMENT[size];
		count = 0;
		del_count = 0;
		for (int i = 0; i < size; i++) {
			marks[i] = BAG_MARK_NULL;
			data[i] = (BAG_ELEMENT)37;
		}    
		int tmp_count = 0;
		// copy the stuff
		if (old_data != NULL) {
			for (int i = 0; i < old_size; i++) {
				if (old_marks[i] == BAG_MARK_PRESENT) {
					tmp_count++;
					Add(old_data[i]);
				}
			}
		}
		// cleanup
		delete [] old_data;
		delete [] old_marks;
	}

public:
	static void extract_int(int e, int &a, int &b, int &c) {
		a = e; b = c = 0; }

protected:

	int hash (int a, int b, int c) const {
		int _a = (a < 0) ? 1 - 2*a : 2*a;
		int _b = (b < 0) ? 1 - 2*b : 2*b;
		int _c = (c < 0) ? 1 - 2*c : 2*c;
		int tmp = 
			LARGE_PRIME_A * _a +
			LARGE_PRIME_B * _b +
			LARGE_PRIME_C * _c;
		// note: abs of the largest negative number is undefined...
		tmp = tmp % size;
		tmp = (tmp<0) ? -tmp : tmp;
		tmp = tmp % size;
		assert (tmp >= 0);
		assert (tmp < size);
		return tmp;
	}

	// ==============
	// REPRESENTATION
	int size;
	int count;
	int del_count;
	BAG_ELEMENT *data;
	enum BAG_ELEMENT_MARK *marks;
	int num_iterators;

	// extract function (an argument to the constructor)
	void (*extract_func)(BAG_ELEMENT, int &a, int &b, int &c);

	// skip function
	inline int skip(int orig, int current) const { 
		assert (current >= 0 && current < size);
		int tmp = (current + 1)%size; 
		assert (current >= 0 && current < size);
		return tmp;
	}

	friend class Iterator<BAG_ELEMENT>;

};

// ======================================================================

template <class ITERATOR_ELEMENT>
class Iterator {

protected:

	// CONSTRUCTOR & DESTRUCTOR
	Iterator(Bag<ITERATOR_ELEMENT> *b) {
		bag = b;
		i = 0; }
	virtual ~Iterator() {}

public:

	// ACCESSOR
	ITERATOR_ELEMENT GetNext() {
		ITERATOR_ELEMENT answer = (ITERATOR_ELEMENT)0;
		while (i < bag->size) {
			if (bag->marks[i] == BAG_MARK_PRESENT) {
				answer = bag->data[i];
				assert (answer != (ITERATOR_ELEMENT)0);
				break;
			}
			i++;      
		}
		i++;
		return answer;
	}

protected:

	Iterator() {}
	friend class Bag<ITERATOR_ELEMENT>;

	// ==============
	// REPRESENTATION
	int i;
	Bag<ITERATOR_ELEMENT> *bag;

};
#endif
