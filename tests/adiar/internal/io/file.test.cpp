#include "../../../test.h"
#include <filesystem>

#include <adiar/internal/io/file.h>
#include <adiar/internal/io/ifstream.h>
#include <adiar/internal/io/iofstream.h>
#include <adiar/internal/io/ofstream.h>

go_bandit([]() {
  describe("adiar/internal/io/file.h , ifstream.h , ofstream.h, iofstream.h", []() {
    // The default folder for temporary files is '/tmp/' on Ubuntu and '/var/tmp/'
    // on Fedora. Both of these are to the OS not on the same drive and so you get
    // a 'cross-device link' error when using std::filesystem::rename(...) to move
    // it to './'.
    //
    // To ensure our tests properly cover BOTH cases, we have to move it inside of
    // the '/tmp/' folder and also to './'.
    //
    // HACK: get the temporary folder itself directly from TPIE.
    const std::string tmp_path  = tpie::tempname::get_actual_path() + "/";
    const std::string curr_path = "./";

    describe("file() [empty]", [&tmp_path, &curr_path]() {
      it("creates a file marked as 'temporary'", []() {
        file<int> f;
        AssertThat(f.is_persistent(), Is().False());
        AssertThat(f.is_temp(), Is().True());
      });

      it("has no content", []() {
        file<int> f;
        AssertThat(f.size(), Is().EqualTo(0u));
        AssertThat(f.empty(), Is().True());
      });

      it("does not (yet) 'exist' on disk", []() {
        file<int> f;

        AssertThat(f.exists(), Is().False());
        AssertThat(std::filesystem::exists(f.path()), Is().False());
      });

      it("'exists' after a 'touch'", []() {
        file<int> f;
        f.touch();

        AssertThat(f.exists(), Is().True());
        AssertThat(std::filesystem::exists(f.path()), Is().True());
      });

      it("is a fresh file", []() {
        file<int> f1;
        file<int> f2;
        AssertThat(f1.path(), Is().Not().EqualTo(f2.path()));
      });

      it("is temporary (removed from disk after destruct)", []() {
        std::string path;
        { // Scope to destruct 'f' early
          file<int> f;
          f.touch();

          path = f.path();
          AssertThat(std::filesystem::exists(path), Is().True());
        }
        AssertThat(std::filesystem::exists(path), Is().False());
      });

      it("can be 'moved' when not existing [./]", [&curr_path]() {
        std::string new_path = curr_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;
        std::string old_path = f.path();

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().False());

        AssertThat(f.can_move(), Is().True());
        f.move(new_path);

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().False());

        f.touch();

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().True());
      });

      it("cannot be 'moved' on-top of an existing file", []() {
        file<int> f1;
        f1.touch();

        file<int> f2;
        f2.touch();

        AssertThat(f1.can_move(), Is().True());
        AssertThrows(runtime_error, f1.move(f2.path()));
      });

      it("can be 'moved' when existing [/tmp/]", [&tmp_path]() {
        std::string new_path = tmp_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;
        f.touch();
        std::string old_path = f.path();

        AssertThat(std::filesystem::exists(old_path), Is().True());
        AssertThat(std::filesystem::exists(new_path), Is().False());

        AssertThat(f.can_move(), Is().True());
        f.move(new_path);

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().True());
      });

      it("can be 'moved' when existing [./]", [&curr_path]() {
        std::string new_path = curr_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;
        f.touch();
        std::string old_path = f.path();

        AssertThat(std::filesystem::exists(old_path), Is().True());
        AssertThat(std::filesystem::exists(new_path), Is().False());

        AssertThat(f.can_move(), Is().True());
        f.move(new_path);

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().True());
      });

      it("is still temporary after move [/tmp/]", [&tmp_path]() {
        std::string new_path = tmp_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        {
          file<int> f;
          f.touch();
          std::string old_path = f.path();

          AssertThat(f.can_move(), Is().True());
          f.move(new_path);

          AssertThat(std::filesystem::exists(new_path), Is().True());

          AssertThat(f.is_persistent(), Is().False());
          AssertThat(f.is_temp(), Is().True());
        }
        AssertThat(std::filesystem::exists(new_path), Is().False());
      });

      it("is still temporary after move [./]", [&curr_path]() {
        std::string new_path = curr_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        {
          file<int> f;
          f.touch();
          std::string old_path = f.path();

          AssertThat(f.can_move(), Is().True());
          f.move(new_path);

          AssertThat(std::filesystem::exists(new_path), Is().True());

          AssertThat(f.is_persistent(), Is().False());
          AssertThat(f.is_temp(), Is().True());
        }
        AssertThat(std::filesystem::exists(new_path), Is().False());
      });

      it("can be made persistent (not removed from disk)", []() {
        std::string path;
        { // Scope to destruct 'f' early
          file<int> f;
          f.touch();

          path = f.path();
          AssertThat(std::filesystem::exists(path), Is().True());

          f.make_persistent();
          AssertThat(f.is_persistent(), Is().True());
          AssertThat(f.is_temp(), Is().False());
        }
        AssertThat(std::filesystem::exists(path), Is().True());

        // Clean up for this test
        std::filesystem::remove(path);
      });

      it("exists on disk after being made persistent", []() {
        std::string path;
        { // Scope to destruct 'f' early
          file<int> f;
          path = f.path();

          AssertThat(f.exists(), Is().False());
          AssertThat(std::filesystem::exists(path), Is().False());
          f.make_persistent();
          AssertThat(f.exists(), Is().True());
          AssertThat(std::filesystem::exists(path), Is().True());
        }
        // Clean up for this test
        if (std::filesystem::exists(path)) { std::filesystem::remove(path); }
      });
    });

    describe("file() + ifstream", []() {
      it("can attach to and detach from an empty file [con-/destructor]", []() {
        file<int> f;
        ifstream<int> fs(f);
      });

      it("can attach to and detach from an empty file [member functions]", []() {
        file<int> f;
        ifstream<int> fs;
        fs.open(f);
      });

      it("remembers it was attached", []() {
        file<int> f;
        ifstream<int> fs(f);
        AssertThat(fs.is_open(), Is().True());
        fs.close();
        AssertThat(fs.is_open(), Is().False());
      });

      it("cannot be pulled from", []() {
        file<int> f;
        ifstream<int> fs(f);

        AssertThat(fs.can_pull(), Is().False());
      });

      it("can be reset", []() {
        file<int> f;
        ifstream<int> fs(f);

        fs.reset();
        AssertThat(fs.is_open(), Is().True());
        AssertThat(fs.can_pull(), Is().False());
      });
    });

    describe("file() + ofstream", [&tmp_path, &curr_path]() {
      it("can attach to and detach from an empty file [con-/destructor]", []() {
        file<int> f;
        ofstream<int> fw(f);
      });

      it("can attach to and detach from an empty file [member functions]", []() {
        file<int> f;
        ofstream<int> fw;
        fw.close();
      });

      it("remembers it was attached", []() {
        file<int> f;
        ofstream<int> fw(f);
        AssertThat(fw.is_open(), Is().True());
        fw.close();
        AssertThat(fw.is_open(), Is().False());
      });

      it("exists after writer attach", []() {
        file<int> f;
        AssertThat(f.exists(), Is().False());

        ofstream<int> fw(f);
        fw.close();

        AssertThat(f.exists(), Is().True());
      });

      it("reports whether elements were pushed", []() {
        file<int> f;
        ofstream<int> fw(f);

        AssertThat(fw.has_pushed(), Is().False());
        AssertThat(fw.empty(), Is().True());
        fw << 1;

        AssertThat(fw.has_pushed(), Is().True());
        AssertThat(fw.empty(), Is().False());

        fw << 2 << 3;

        AssertThat(fw.has_pushed(), Is().True());
        AssertThat(fw.empty(), Is().False());
      });

      it("changes size when writing content to file [1]", []() {
        file<int> f;

        ofstream<int> fw(f);

        AssertThat(fw.size(), Is().EqualTo(0u));
        fw << 1 << 2;
        AssertThat(fw.size(), Is().EqualTo(2u));
        fw << 3;
        AssertThat(fw.size(), Is().EqualTo(3u));
        fw.close();

        AssertThat(f.size(), Is().EqualTo(3u));
      });

      it("changes size when writing content to file [2]", []() {
        file<int> f;

        ofstream<int> fw(f);
        AssertThat(fw.size(), Is().EqualTo(0u));
        fw << 42;
        AssertThat(fw.size(), Is().EqualTo(1u));
        fw << 21;
        AssertThat(fw.size(), Is().EqualTo(2u));
        fw.close();

        AssertThat(f.size(), Is().EqualTo(2u));
      });

      it("can be 'moved' after write [/tmp/]", [&tmp_path]() {
        std::string new_path = tmp_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;

        ofstream<int> fw(f);
        fw << 42 << 21;
        fw.close();

        std::string old_path = f.path();

        AssertThat(std::filesystem::exists(old_path), Is().True());
        AssertThat(std::filesystem::exists(new_path), Is().False());

        AssertThat(f.can_move(), Is().True());
        f.move(new_path);

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().True());
      });

      it("can be 'moved' after write [./]", [&curr_path]() {
        std::string new_path = curr_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;

        ofstream<int> fw(f);
        fw << 42 << 21;
        fw.close();

        std::string old_path = f.path();

        AssertThat(std::filesystem::exists(old_path), Is().True());
        AssertThat(std::filesystem::exists(new_path), Is().False());

        AssertThat(f.can_move(), Is().True());
        f.move(new_path);

        AssertThat(std::filesystem::exists(old_path), Is().False());
        AssertThat(std::filesystem::exists(new_path), Is().True());
      });
    });

    describe("file() + ifstream + ofstream", [&tmp_path, &curr_path]() {
      it("can read written content [1]", []() {
        file<int> f;

        ofstream<int> fw(f);
        fw << 1 << 2 << 3;
        fw.close();
        AssertThat(fw.is_open(), Is().False());

        ifstream<int, false> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(1));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(3));
        AssertThat(fs.can_pull(), Is().False());
        fs.close();
      });

      it("can read written content [2]", []() {
        file<int> f;

        ofstream<int> fw(f);
        fw << 42 << 21;
        fw.close();
        AssertThat(fw.is_open(), Is().False());

        ifstream<int, false> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(42));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(21));
        AssertThat(fs.can_pull(), Is().False());
        fs.close();
      });

      it("can read written content in reverse [1]", []() {
        file<int> f;

        ofstream<int> fw(f);
        fw << 1 << 2 << 3;
        fw.close();
        AssertThat(fw.is_open(), Is().False());

        ifstream<int, true> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(3));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(1));
        AssertThat(fs.can_pull(), Is().False());
        fs.close();
      });

      it("can read written content in reverse [2]", []() {
        file<int> f;

        ofstream<int> fw(f);
        fw << 42 << 21;
        fw.close();
        AssertThat(fw.is_open(), Is().False());

        ifstream<int, true> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(21));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(42));
        AssertThat(fs.can_pull(), Is().False());
        fs.close();
      });

      it("can seek existing elements [forward]", []() {
        file<int> f;

        ofstream<int> fw(f);
        fw << 1 << 2 << 3 << 4 << 5 << 6;
        fw.close();
        AssertThat(fw.is_open(), Is().False());

        ifstream<int> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(2), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(2), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(3), Is().EqualTo(3));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(6), Is().EqualTo(6));
        AssertThat(fs.can_pull(), Is().True());
      });

      it("can seek existing elements [reverse]", []() {
        file<int> f;

        ofstream<int> fw(f);
        fw << 6 << 5 << 4 << 3 << 2 << 1;
        fw.close();
        AssertThat(fw.is_open(), Is().False());

        ifstream<int, true> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(2), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(2), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(3), Is().EqualTo(3));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(6), Is().EqualTo(6));
        AssertThat(fs.can_pull(), Is().True());
      });

      it("can seek non-existing element [forward]", []() {
        file<int> f;

        ofstream<int> fw(f);
        fw << 1 << 2 << 3 << 5 << 6;
        fw.close();
        AssertThat(fw.is_open(), Is().False());

        ifstream<int> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(2), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(4), Is().EqualTo(5));
        AssertThat(fs.can_pull(), Is().True());
      });

      it("can seek non-existing element [reverse]", []() {
        file<int> f;

        ofstream<int> fw(f);
        fw << 6 << 5 << 3 << 2 << 1;
        fw.close();
        AssertThat(fw.is_open(), Is().False());

        ifstream<int, true> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(2), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(4), Is().EqualTo(5));
        AssertThat(fs.can_pull(), Is().True());
      });

      it("can seek past end [forward]", []() {
        file<int> f;

        ofstream<int> fw(f);
        fw << 1 << 2 << 3 << 5 << 6;
        fw.close();
        AssertThat(fw.is_open(), Is().False());

        ifstream<int> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(3), Is().EqualTo(3));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(8), Is().EqualTo(6));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(7), Is().EqualTo(6));
        AssertThat(fs.can_pull(), Is().True());
      });

      it("can seek past end [reverse]", []() {
        file<int> f;

        ofstream<int> fw(f);
        fw << 6 << 5 << 3 << 2 << 1;
        fw.close();
        AssertThat(fw.is_open(), Is().False());

        ifstream<int, true> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(3), Is().EqualTo(3));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(8), Is().EqualTo(6));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.seek(7), Is().EqualTo(6));
        AssertThat(fs.can_pull(), Is().True());
      });

      it("can sort written content", []() {
        file<int> f;

        ofstream<int> fw(f);
        fw << 42 << 2 << 32 << 21;
        fw.sort<std::less<>>();
        fw.close();
        AssertThat(fw.is_open(), Is().False());

        ifstream<int> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(21));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(32));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(42));
        AssertThat(fs.can_pull(), Is().False());
        fs.close();
      });

      it("can read from 'moved' file [/tmp/]", [&tmp_path]() {
        std::string new_path = tmp_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;

        ofstream<int> fw(f);
        fw << 12 << 9 << 1;
        fw.close();

        AssertThat(f.can_move(), Is().True());
        f.move(new_path);

        ifstream<int, false> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(12));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(9));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(1));
        AssertThat(fs.can_pull(), Is().False());
        fs.close();
      });

      it("can read from 'moved' file [./]", [&curr_path]() {
        std::string new_path = curr_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;

        ofstream<int> fw(f);
        fw << 12 << 9 << 1;
        fw.close();

        AssertThat(f.can_move(), Is().True());
        f.move(new_path);

        ifstream<int, false> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(12));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(9));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(1));
        AssertThat(fs.can_pull(), Is().False());
        fs.close();
      });

      it("can read from 'moved' file in reverse [/tmp/]", [&tmp_path]() {
        std::string new_path = tmp_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;

        ofstream<int> fw(f);
        fw << 8 << 9 << 4 << 2;
        fw.close();

        AssertThat(f.can_move(), Is().True());
        f.move(new_path);

        ifstream<int, true> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(4));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(9));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(8));
        AssertThat(fs.can_pull(), Is().False());
        fs.close();
      });

      it("can read from 'moved' file in reverse [./]", [&curr_path]() {
        std::string new_path = curr_path + "after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f;

        ofstream<int> fw(f);
        fw << 8 << 9 << 4 << 2;
        fw.close();

        AssertThat(f.can_move(), Is().True());
        f.move(new_path);

        ifstream<int, true> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(4));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(9));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(8));
        AssertThat(fs.can_pull(), Is().False());
        fs.close();
      });
    });

    describe("iofstream", []() {
      iofstream<int> s;
      s.open();

      it("temporary file is initially empty", [&s]() {
        AssertThat(s.size(), Is().EqualTo(0u));
        AssertThat(s.has_prev(), Is().False());
        AssertThat(s.has_next(), Is().False());
      });

      it("can write elements", [&s]() {
        AssertThat(s.size(), Is().EqualTo(0u));
        s.write(1);
        AssertThat(s.size(), Is().EqualTo(1u));
        s.write(2);
        AssertThat(s.size(), Is().EqualTo(2u));
        s.write(2);
        AssertThat(s.size(), Is().EqualTo(3u));
        s.write(4);
        AssertThat(s.size(), Is().EqualTo(4u));
      });

      it("can read elements backwards", [&s]() {
        AssertThat(s.size(), Is().EqualTo(4u));
        AssertThat(s.has_next(), Is().False());

        AssertThat(s.has_prev(), Is().True());
        AssertThat(s.prev(), Is().EqualTo(4));
        AssertThat(s.has_prev(), Is().True());
        AssertThat(s.prev(), Is().EqualTo(2));
        AssertThat(s.has_prev(), Is().True());
        AssertThat(s.prev(), Is().EqualTo(2));
        AssertThat(s.has_prev(), Is().True());
        AssertThat(s.prev(), Is().EqualTo(1));
        AssertThat(s.has_prev(), Is().False());
      });

      it("can read elements forwards", [&s]() {
        AssertThat(s.size(), Is().EqualTo(4u));
        AssertThat(s.has_prev(), Is().False());

        AssertThat(s.has_next(), Is().True());
        AssertThat(s.next(), Is().EqualTo(1));
        AssertThat(s.has_next(), Is().True());
        AssertThat(s.next(), Is().EqualTo(2));
        AssertThat(s.has_next(), Is().True());
        AssertThat(s.next(), Is().EqualTo(2));
        AssertThat(s.has_next(), Is().True());
        AssertThat(s.next(), Is().EqualTo(4));
        AssertThat(s.has_next(), Is().False());
      });

      it("can seek to the beginning", [&s]() {
        AssertThat(s.size(), Is().EqualTo(4u));
        AssertThat(s.has_prev(), Is().True());
        AssertThat(s.has_next(), Is().False());

        s.seek_begin();

        AssertThat(s.has_prev(), Is().False());
        AssertThat(s.has_next(), Is().True());
        AssertThat(s.next(), Is().EqualTo(1));
      });

      it("can seek to the end", [&s]() {
        AssertThat(s.size(), Is().EqualTo(4u));
        AssertThat(s.has_prev(), Is().True());
        AssertThat(s.has_next(), Is().True());

        s.seek_end();

        AssertThat(s.has_prev(), Is().True());
        AssertThat(s.has_next(), Is().False());
        AssertThat(s.prev(), Is().EqualTo(4));
      });

      it("can seek to offset 2", [&s]() {
        AssertThat(s.size(), Is().EqualTo(4u));
        AssertThat(s.has_prev(), Is().True());
        AssertThat(s.has_next(), Is().True());

        s.seek(1);

        AssertThat(s.has_prev(), Is().True());
        AssertThat(s.has_next(), Is().True());
        AssertThat(s.next(), Is().EqualTo(2));
        AssertThat(s.has_next(), Is().True());
        AssertThat(s.next(), Is().EqualTo(2));
        AssertThat(s.has_next(), Is().True());
        AssertThat(s.next(), Is().EqualTo(4));
        AssertThat(s.has_next(), Is().False());
      });

      it("can overwrite a value", [&s]() {
        AssertThat(s.size(), Is().EqualTo(4u));
        AssertThat(s.has_prev(), Is().True());
        AssertThat(s.has_next(), Is().False());

        s.seek_begin();

        s.write(3);
        AssertThat(s.size(), Is().EqualTo(4u));
        s.write(1);
        AssertThat(s.size(), Is().EqualTo(4u));
        s.write(5);
        AssertThat(s.size(), Is().EqualTo(4u));
        s.write(2);
        AssertThat(s.size(), Is().EqualTo(4u));
        s.write(4);
        AssertThat(s.size(), Is().EqualTo(5u));
      });
    });

    describe("file() + iofstream", []() {
      file<int> f;

      it("can write to file", [&f]() {
        iofstream<int> s(f);

        AssertThat(s.size(), Is().EqualTo(0u));
        s.write(1);
        AssertThat(s.size(), Is().EqualTo(1u));
        s.write(2);
        AssertThat(s.size(), Is().EqualTo(2u));
        s.write(3);
        AssertThat(s.size(), Is().EqualTo(3u));
      });

      it("can reopen file and read forwards", [&f]() {
        iofstream<int> s(f);

        AssertThat(s.has_next(), Is().True());
        AssertThat(s.next(), Is().EqualTo(1));
        AssertThat(s.has_next(), Is().True());
        AssertThat(s.next(), Is().EqualTo(2));
        AssertThat(s.has_next(), Is().True());
        AssertThat(s.next(), Is().EqualTo(3));
        AssertThat(s.has_next(), Is().False());
      });

      it("can reopen file, seek to the end, and read backwards", [&f]() {
        iofstream<int> s(f);
        s.seek_end();

        AssertThat(s.has_prev(), Is().True());
        AssertThat(s.prev(), Is().EqualTo(3));
        AssertThat(s.has_prev(), Is().True());
        AssertThat(s.prev(), Is().EqualTo(2));
        AssertThat(s.has_prev(), Is().True());
        AssertThat(s.prev(), Is().EqualTo(1));
        AssertThat(s.has_prev(), Is().False());
      });

      it("can reopen file and overwrite its content", [&f]() {
        iofstream<int> s(f);

        AssertThat(s.size(), Is().EqualTo(3u));
        s.write(4);
        AssertThat(s.size(), Is().EqualTo(3u));

        s.seek_begin();

        AssertThat(s.has_next(), Is().True());
        AssertThat(s.next(), Is().EqualTo(4));
        AssertThat(s.has_next(), Is().True());
        AssertThat(s.next(), Is().EqualTo(2));
        AssertThat(s.has_next(), Is().True());
        AssertThat(s.next(), Is().EqualTo(3));
        AssertThat(s.has_next(), Is().False());
      });

      it("can reopen file and write to its end", [&f]() {
        iofstream<int> s(f);
        s.seek_end();

        AssertThat(s.size(), Is().EqualTo(3u));
        s.write(5);
        AssertThat(s.size(), Is().EqualTo(4u));

        AssertThat(s.has_prev(), Is().True());
        AssertThat(s.prev(), Is().EqualTo(5));
        AssertThat(s.has_prev(), Is().True());
        AssertThat(s.prev(), Is().EqualTo(3));
        AssertThat(s.has_prev(), Is().True());
        AssertThat(s.prev(), Is().EqualTo(2));
        AssertThat(s.has_prev(), Is().True());
        AssertThat(s.prev(), Is().EqualTo(4));
        AssertThat(s.has_prev(), Is().False());
      });
    });

    describe("file(path)", []() {
      it("throws exception on path to non-existing file",
         []() { AssertThrows(runtime_error, file<int>("./non-existing-file.adiar")); });

      std::string path;
      {
        file<int> f;
        f.touch();

        path = f.path();
        f.make_persistent();
      }

      it("can reopen existing file", [&path]() {
        file<int> f(path);
        AssertThat(f.path(), Is().EqualTo(path));

        AssertThat(f.exists(), Is().True());
        AssertThat(f.size(), Is().EqualTo(0u));
      });

      it("is marked persistent", [&path]() {
        file<int> f(path);
        AssertThat(f.is_persistent(), Is().True());
        AssertThat(f.is_temp(), Is().False());
      });

      it("is unchanged after marking it persistent once more", [&path]() {
        {
          file<int> f(path);
          f.make_persistent();

          AssertThat(f.is_persistent(), Is().True());
          AssertThat(f.is_temp(), Is().False());
        }
        AssertThat(std::filesystem::exists(path), Is().True());
      });

      it("cannot be 'moved'", [&path]() {
        std::string new_path = "./after-move-path.adiar";
        if (std::filesystem::exists(new_path)) {
          // Clean up after prior test run
          std::filesystem::remove(new_path);
        }

        file<int> f(path);
        std::string old_path = f.path();

        AssertThat(f.can_move(), Is().False());
        AssertThrows(runtime_error, f.move(new_path));
      });

      // Clean up for above tests
      if (std::filesystem::exists(path)) { std::filesystem::remove(path); }

      {
        file<int> f;

        ofstream<int> fw(f);
        fw << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9;
        fw.close();

        f.move(path);
        f.make_persistent();
      }

      it("can reopen a non-empty persisted file", [&path]() { file<int> f(path); });

      it("has expected size after reopening a persisted non-empty file", [&path]() {
        file<int> f(path);

        AssertThat(f.empty(), Is().False());
        AssertThat(f.size(), Is().EqualTo(10u));
      });

      it("can read content with a stream", [&path]() {
        file<int> f(path);
        ifstream<int> fs(f);

        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(0));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(1));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(3));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(4));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(5));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(6));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(7));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(8));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(9));
        AssertThat(fs.can_pull(), Is().False());
      });

      it("can read content in reverse with a stream", [&path]() {
        file<int> f(path);
        ifstream<int, true> fs(f);

        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(9));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(8));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(7));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(6));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(5));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(4));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(3));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(1));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(0));
        AssertThat(fs.can_pull(), Is().False());
      });

      it("cannot reattach a writer to a persisted file", [&path]() {
        file<int> f(path);
        ofstream<int> fw;
        AssertThrows(runtime_error, fw.open(f));
      });

      // Clean up for above tests
      if (std::filesystem::exists(path)) { std::filesystem::remove(path); }
    });

    describe("file.sort(const pred_t&)", []() {
      it("can sort non-existing file", []() {
        file<int> f;
        AssertThat(f.exists(), Is().False());

        f.sort<std::less<int>>();
        AssertThat(f.exists(), Is().False());

        ifstream<int> fs(f);
        AssertThat(fs.can_pull(), Is().False());
        fs.close();
      });

      it("can sort existing empty file", []() {
        file<int> f;
        f.touch();
        AssertThat(f.exists(), Is().True());

        f.sort<std::less<int>>();
        AssertThat(f.exists(), Is().True());

        ifstream<int> fs(f);
        AssertThat(fs.can_pull(), Is().False());
        fs.close();
      });

      it("can sort non-empty file [1]", []() {
        file<int> f;

        ofstream<int> fw(f);
        fw << 8 << 9 << 4 << 2;
        fw.close();

        f.sort<std::less<int>>();

        ifstream<int> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(4));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(8));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(9));
        AssertThat(fs.can_pull(), Is().False());
        fs.close();
      });

      it("can sort non-empty file [2]", []() {
        file<int> f;

        ofstream<int> fw(f);
        fw << 42 << -1 << 8 << 21 << 8 << 3;
        fw.close();

        f.sort<std::less<int>>();

        ifstream<int> fs(f);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(-1));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(3));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(8));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(8));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(21));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(42));
        AssertThat(fs.can_pull(), Is().False());
        fs.close();
      });

      it("cannot sort a persisted empty file", []() {
        std::string path;
        { // Scope to destruct 'f' early
          file<int> f;
          f.touch();

          path = f.path();
          f.make_persistent();
          AssertThat(std::filesystem::exists(path), Is().True());

          AssertThrows(runtime_error, f.sort<std::less<int>>());
        }
        AssertThat(std::filesystem::exists(path), Is().True());

        // Clean up for this test
        std::filesystem::remove(path);
      });

      it("cannot sort a persisted non-empty file", []() {
        std::string path;
        { // Scope to destruct 'f' early
          file<int> f;
          f.touch();

          ofstream<int> fw(f);
          fw << -1 << 8 << 3;
          fw.close();

          path = f.path();
          f.make_persistent();
          AssertThat(std::filesystem::exists(path), Is().True());

          AssertThrows(runtime_error, f.sort<std::less<int>>());
        }
        AssertThat(std::filesystem::exists(path), Is().True());

        { // Check is not sorted
          file<int> f(path);
          ifstream<int> fs(f);
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(-1));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(8));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(3));
          AssertThat(fs.can_pull(), Is().False());
          fs.close();
        }

        // Clean up for this test
        std::filesystem::remove(path);
      });
    });

    describe("file::copy(const file&)", []() {
      it("can copy over non-existing file", []() {
        file<int> f1;
        AssertThat(f1.exists(), Is().False());

        file<int> f2 = file<int>::copy(f1);
        AssertThat(f2.exists(), Is().False());

        AssertThat(f2.path(), Is().Not().EqualTo(f1.path()));
      });

      it("is unaffected by original file being 'touched'", []() {
        file<int> f1;
        file<int> f2 = file<int>::copy(f1);

        f1.touch();
        AssertThat(f2.exists(), Is().False());
      });

      it("can copy over an existing file [empty]", []() {
        file<int> f1;
        f1.touch();
        AssertThat(f1.exists(), Is().True());

        file<int> f2 = file<int>::copy(f1);
        AssertThat(f2.exists(), Is().True());
        AssertThat(f2.path(), Is().Not().EqualTo(f1.path()));
      });

      it("can copy over an existing file [non-empty, 1]", []() {
        file<int> f1;
        ofstream<int> fw(f1);

        fw << 21 << 42 << 21;
        fw.close();
        AssertThat(f1.exists(), Is().True());
        AssertThat(f1.size(), Is().EqualTo(3u));

        file<int> f2 = file<int>::copy(f1);

        AssertThat(f2.exists(), Is().True());
        AssertThat(f2.size(), Is().EqualTo(3u));
        AssertThat(f2.path(), Is().Not().EqualTo(f1.path()));

        ifstream<int> fs(f2);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(21));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(42));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(21));
        AssertThat(fs.can_pull(), Is().False());
        fs.close();
      });

      it("can copy over an existing file [non-empty, 2]", []() {
        file<int> f1;
        ofstream<int> fw(f1);

        fw << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9;
        fw.close();
        AssertThat(f1.exists(), Is().True());
        AssertThat(f1.size(), Is().EqualTo(10u));

        file<int> f2 = file<int>::copy(f1);

        AssertThat(f2.exists(), Is().True());
        AssertThat(f2.size(), Is().EqualTo(10u));
        AssertThat(f2.path(), Is().Not().EqualTo(f1.path()));

        ifstream<int> fs(f2);
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(0));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(1));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(2));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(3));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(4));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(5));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(6));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(7));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(8));
        AssertThat(fs.can_pull(), Is().True());
        AssertThat(fs.pull(), Is().EqualTo(9));
        AssertThat(fs.can_pull(), Is().False());
        fs.close();
      });

      it("is temporary if original file is temporary [empty]", []() {
        file<int> f1;
        f1.touch();

        file<int> f2 = file<int>::copy(f1);
        AssertThat(f2.exists(), Is().True());
        AssertThat(f2.is_persistent(), Is().False());
        AssertThat(f2.is_temp(), Is().True());
      });

      it("is temporary if original file is temporary [non-empty]", []() {
        file<int> f1;
        ofstream<int> fw(f1);
        fw << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9;
        fw.close();
        AssertThat(f1.exists(), Is().True());
        AssertThat(f1.size(), Is().EqualTo(10u));

        file<int> f2 = file<int>::copy(f1);
        AssertThat(f2.exists(), Is().True());
        AssertThat(f2.is_persistent(), Is().False());
        AssertThat(f2.is_temp(), Is().True());
      });

      it("is temporary if original file is persistent [empty]", []() {
        std::string path;
        {
          file<int> f1;
          f1.touch();
          f1.make_persistent();

          file<int> f2 = file<int>::copy(f1);
          AssertThat(f2.exists(), Is().True());
          AssertThat(f2.is_persistent(), Is().False());
          AssertThat(f2.is_temp(), Is().True());

          // clean up after f1
          path = f1.path();
        }
        std::filesystem::remove(path);
      });

      it("is temporary if original file is persistent [non-empty]", []() {
        std::string path;
        {
          file<int> f1;
          ofstream<int> fw(f1);
          fw << 0 << 1 << 2 << 3 << 4;
          fw.close();
          f1.make_persistent();

          file<int> f2 = file<int>::copy(f1);
          AssertThat(f2.exists(), Is().True());
          AssertThat(f2.is_persistent(), Is().False());
          AssertThat(f2.is_temp(), Is().True());

          // clean up after f1
          path = f1.path();
        }
        std::filesystem::remove(path);
      });
    });
  });
});
