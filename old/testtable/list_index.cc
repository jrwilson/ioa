#include <table.hh>

#include <assert.h>

int
main (int argc, char* argv[]) {
  {
    Table<int> table;
    ListIndex<Table<int> > index (table);
    assert (index.empty ());
    assert (index.size () == 0);
  }

  {
    Table<int> table;
    ListIndex<Table<int> > index (table);
    for (int i = 0; i < 10; ++i) {
      index.push_back (i);
    }
    assert (!index.empty ());
    assert (index.size () == 10);
    int i;
    i = 0;
    for (ListIndex<Table<int> >::const_iterator pos = index.begin ();
    	 pos != index.end ();
    	 ++pos) {
      assert (*pos == i);
      ++i;
    }
    i = 9;
    for (ListIndex<Table<int> >::const_reverse_iterator pos = index.rbegin ();
    	 pos != index.rend ();
    	 ++pos) {
      assert (*pos == i);
      --i;
    }

    assert (index.front () == 0);
    assert (index.back () == 9);
  }

  return 0;
}
