#include "../../../test.h"
#include <array>
#include <filesystem>

go_bandit([]() {
  describe("adiar/internal/io/shared_file_ptr.h", []() {
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

    describe("shared_file<int>", [&tmp_path, &curr_path]() {
      // TODO: make_shared_file

      it("is default-constructed as non-nullptr", []() {
        shared_file<int> f;
        AssertThat(f.get(), Is().Not().Null());
      });

      it("is default-constructed as a separate temporary and empty file object", []() {
        shared_file<int> f1;
        shared_file<int> f2;
        AssertThat(f1.get(), Is().Not().EqualTo(f2.get()));
        AssertThat(f1->path(), Is().Not().EqualTo(f2->path()));

        AssertThat(f1->is_persistent(), Is().False());
        AssertThat(f1->is_temp(), Is().True());
        AssertThat(f1->empty(), Is().True());

        AssertThat(f2->is_persistent(), Is().False());
        AssertThat(f2->is_temp(), Is().True());
        AssertThat(f2->empty(), Is().True());
      });

      it("is copy-constructed to share the file object", []() {
        shared_file<int> f1;
        shared_file<int> f2(f1);
        AssertThat(f1.get(), Is().EqualTo(f2.get()));
      });

      it("shares the file object after assignment", []() {
        shared_file<int> f1;
        shared_file<int> f2 = f1;
        AssertThat(f1.get(), Is().EqualTo(f2.get()));
      });

      it("has temporary files deleted from disk with destructor", []() {
        std::string path;

        {
          shared_file<int> f;
          path = f->path();
          AssertThat(std::filesystem::exists(path), Is().False());
          f->touch();
          AssertThat(std::filesystem::exists(path), Is().True());
          AssertThat(f->is_temp(), Is().True());
          AssertThat(f->exists(), Is().True());
        }
        AssertThat(std::filesystem::exists(path), Is().False());
      });

      it("can 'reset' into a nullptr and trigger deletion of file on disk", []() {
        shared_file<int> f;
        f->touch();
        const std::string path = f->path();

        AssertThat(std::filesystem::exists(path), Is().True());
        f.reset();

        AssertThat(std::filesystem::exists(path), Is().False());
        AssertThat(f.get(), Is().Null());
      });

      it("first deletes file on disk when reference count reaches 0 [.reset()]", []() {
        shared_file<int> f1;
        f1->touch();
        const std::string path = f1->path();

        AssertThat(std::filesystem::exists(path), Is().True());

        shared_file<int> f2(f1);

        AssertThat(std::filesystem::exists(path), Is().True());

        f1.reset();

        AssertThat(std::filesystem::exists(path), Is().True());

        f2.reset();

        AssertThat(std::filesystem::exists(path), Is().False());
      });

      it("first deletes file on disk when reference count reaches 0 [scope]", []() {
        std::string path;
        {
          shared_file<int> f1;
          f1->touch();
          path = f1->path();

          AssertThat(std::filesystem::exists(path), Is().True());
          {
            shared_file<int> f2 = f1;
            AssertThat(std::filesystem::exists(path), Is().True());
          }
          AssertThat(std::filesystem::exists(path), Is().True());
        }
        AssertThat(std::filesystem::exists(path), Is().False());
      });

      describe("w/ ofstream + ifstream", []() {
        it("ofstream is part of reference counting", []() {
          std::string path;

          ofstream<int> fw;
          {
            shared_file<int> f;
            path = f->path();
            fw.open(f);
            AssertThat(std::filesystem::exists(path), Is().True());
          }
          AssertThat(std::filesystem::exists(path), Is().True());
          fw.close();
          AssertThat(std::filesystem::exists(path), Is().False());
        });

        it("ifstream is part of reference counting", []() {
          std::string path;

          ifstream<int> fs;
          {
            shared_file<int> f;
            path = f->path();
            fs.open(f);
            AssertThat(std::filesystem::exists(path), Is().True());
          }
          AssertThat(std::filesystem::exists(path), Is().True());
          fs.close();
          AssertThat(std::filesystem::exists(path), Is().False());
        });

        it("can read written content", []() {
          shared_file<int> f;
          {
            ofstream<int> fw(f);
            fw << 1 << 2 << 3 << 4;
          }
          {
            ifstream<int, false> fs(f);
            AssertThat(fs.can_pull(), Is().True());
            AssertThat(fs.pull(), Is().EqualTo(1));
            AssertThat(fs.can_pull(), Is().True());
            AssertThat(fs.pull(), Is().EqualTo(2));
            AssertThat(fs.can_pull(), Is().True());
            AssertThat(fs.pull(), Is().EqualTo(3));
            AssertThat(fs.can_pull(), Is().True());
            AssertThat(fs.pull(), Is().EqualTo(4));
            AssertThat(fs.can_pull(), Is().False());
          }
        });

        it("can read written content in reverse", []() {
          shared_file<int> f;
          {
            ofstream<int> fw(f);
            fw << 21 << 42 << 21 << 84;
          }
          {
            ifstream<int, true> fs(f);
            AssertThat(fs.can_pull(), Is().True());
            AssertThat(fs.pull(), Is().EqualTo(84));
            AssertThat(fs.can_pull(), Is().True());
            AssertThat(fs.pull(), Is().EqualTo(21));
            AssertThat(fs.can_pull(), Is().True());
            AssertThat(fs.pull(), Is().EqualTo(42));
            AssertThat(fs.can_pull(), Is().True());
            AssertThat(fs.pull(), Is().EqualTo(21));
            AssertThat(fs.can_pull(), Is().False());
          }
        });

        // TODO: file content header
      });

      describe("::copy(...)", []() {
        it("can copy over empty file", []() {
          shared_file<int> f1;

          AssertThat(f1->exists(), Is().False());
          f1->touch();
          AssertThat(f1->exists(), Is().True());
          AssertThat(f1->empty(), Is().True());

          shared_file<int> f2 = shared_file<int>::copy(f1);

          AssertThat(f2->exists(), Is().True());
          AssertThat(f1->path(), Is().Not().EqualTo(f2->path()));
        });

        it("can copy over non-empty file", []() {
          shared_file<int> f1;
          {
            ofstream<int> fw(f1);
            fw << 1 << 2 << 4 << 8 << 16;
          }

          shared_file<int> f2 = shared_file<int>::copy(f1);

          // Check path
          AssertThat(f2->path(), Is().Not().EqualTo(f1->path()));

          // Check content
          {
            ifstream<int> fs(f2);
            AssertThat(fs.can_pull(), Is().True());
            AssertThat(fs.pull(), Is().EqualTo(1));
            AssertThat(fs.can_pull(), Is().True());
            AssertThat(fs.pull(), Is().EqualTo(2));
            AssertThat(fs.can_pull(), Is().True());
            AssertThat(fs.pull(), Is().EqualTo(4));
            AssertThat(fs.can_pull(), Is().True());
            AssertThat(fs.pull(), Is().EqualTo(8));
            AssertThat(fs.can_pull(), Is().True());
            AssertThat(fs.pull(), Is().EqualTo(16));
            AssertThat(fs.can_pull(), Is().False());
          }
        });
      });

      it("can move, persist and reopen a file [/tmp/]", [&tmp_path]() {
        std::string file_path = tmp_path + "persisted-shared-path.adiar";

        // Clean up after prior test run
        if (std::filesystem::exists(file_path)) { std::filesystem::remove(file_path); }

        { // Create a persisted file
          shared_file<int> f;

          ofstream<int> fw(f);
          fw << 0 << 2 << 4 << 6 << 8;

          // TODO: header file content
          fw.close();

          f->move(file_path);
          f->make_persistent();
        }

        {
          shared_file<int> f(file_path);

          ifstream<int> fs(f);
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(0));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(2));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(4));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(6));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(8));
          AssertThat(fs.can_pull(), Is().False());

          // TODO: header file content
        }

        // Clean up of this test
        if (std::filesystem::exists(file_path)) { std::filesystem::remove(file_path); }
      });

      it("can move, persist and reopen file [./]", [&curr_path]() {
        std::string file_path = curr_path + "persisted-shared-path.adiar";

        // Clean up after prior test run
        if (std::filesystem::exists(file_path)) { std::filesystem::remove(file_path); }

        { // Create a persisted file
          shared_file<int> f;

          ofstream<int> fw(f);
          fw << 1 << 3 << 5 << 7;

          // TODO: header file content
          fw.close();

          f->move(file_path);
          f->make_persistent();
        }

        {
          shared_file<int> f(file_path);

          ifstream<int> fs(f);
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(1));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(3));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(5));
          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(7));
          AssertThat(fs.can_pull(), Is().False());

          // TODO: header file content
        }

        // Clean up of this test
        if (std::filesystem::exists(file_path)) { std::filesystem::remove(file_path); }
      });
    });

    describe("shared_levelized_file<int>()", [&tmp_path, &curr_path]() {
      // TODO: make_shared_levelized_file

      it("is default constructed as a non-nullptr", []() {
        shared_levelized_file<int> lf;
        AssertThat(lf.get(), Is().Not().Null());
      });

      it("is default constructed as a separate temporary and empty file object", []() {
        shared_levelized_file<int> lf1;
        shared_levelized_file<int> lf2;
        AssertThat(lf1.get(), Is().Not().EqualTo(lf2.get()));

        AssertThat(lf1->is_persistent(), Is().False());
        AssertThat(lf1->is_temp(), Is().True());
        AssertThat(lf1->empty(), Is().True());

        AssertThat(lf2->is_persistent(), Is().False());
        AssertThat(lf2->is_temp(), Is().True());
        AssertThat(lf2->empty(), Is().True());

        const auto paths1 = lf1->paths();
        const auto paths2 = lf2->paths();

        AssertThat(paths1[0], Is().Not().EqualTo(paths2[0]));
        AssertThat(paths1[0], Is().Not().EqualTo(paths2[1]));
        AssertThat(paths1[0], Is().Not().EqualTo(paths2[2]));

        AssertThat(paths1[1], Is().Not().EqualTo(paths2[0]));
        AssertThat(paths1[1], Is().Not().EqualTo(paths2[1]));
        AssertThat(paths1[1], Is().Not().EqualTo(paths2[2]));

        AssertThat(paths1[2], Is().Not().EqualTo(paths2[0]));
        AssertThat(paths1[2], Is().Not().EqualTo(paths2[1]));
        AssertThat(paths1[2], Is().Not().EqualTo(paths2[2]));
      });

      it("is copy-constructed to share the same levelized file object", []() {
        shared_levelized_file<int> lf1;
        shared_levelized_file<int> lf2(lf1);
        AssertThat(lf1.get(), Is().EqualTo(lf2.get()));
      });

      it("shares the same levelized file object after assignment", []() {
        shared_levelized_file<int> lf1;
        shared_levelized_file<int> lf2 = lf1;
        AssertThat(lf1.get(), Is().EqualTo(lf2.get()));
      });

      it("has temporary files deleted from disk with destructor", []() {
        std::array<std::string, 2u + 1u> paths;
        {
          shared_levelized_file<int> lf;
          paths = lf->paths();
          lf->touch();

          AssertThat(std::filesystem::exists(paths[0]), Is().True());
          AssertThat(std::filesystem::exists(paths[1]), Is().True());
          AssertThat(std::filesystem::exists(paths[2]), Is().True());
        }
        AssertThat(std::filesystem::exists(paths[0]), Is().False());
        AssertThat(std::filesystem::exists(paths[1]), Is().False());
        AssertThat(std::filesystem::exists(paths[2]), Is().False());
      });

      it("can 'reset' into a nullptr and trigger deletion of file on disk", []() {
        shared_levelized_file<int> lf;
        lf->touch();
        const auto paths = lf->paths();

        AssertThat(std::filesystem::exists(paths[0]), Is().True());
        AssertThat(std::filesystem::exists(paths[1]), Is().True());
        AssertThat(std::filesystem::exists(paths[2]), Is().True());

        lf.reset();

        AssertThat(std::filesystem::exists(paths[0]), Is().False());
        AssertThat(std::filesystem::exists(paths[1]), Is().False());
        AssertThat(std::filesystem::exists(paths[2]), Is().False());
        AssertThat(lf.get(), Is().Null());
      });

      it("first deletes file on disk when reference count reaches 0 [.reset()]", []() {
        shared_levelized_file<int> lf1;
        lf1->touch();
        const auto paths = lf1->paths();

        AssertThat(std::filesystem::exists(paths[0]), Is().True());
        AssertThat(std::filesystem::exists(paths[1]), Is().True());
        AssertThat(std::filesystem::exists(paths[2]), Is().True());

        shared_levelized_file<int> lf2(lf1);

        AssertThat(std::filesystem::exists(paths[0]), Is().True());
        AssertThat(std::filesystem::exists(paths[1]), Is().True());
        AssertThat(std::filesystem::exists(paths[2]), Is().True());

        lf1.reset();

        AssertThat(std::filesystem::exists(paths[0]), Is().True());
        AssertThat(std::filesystem::exists(paths[1]), Is().True());
        AssertThat(std::filesystem::exists(paths[2]), Is().True());

        lf2.reset();

        AssertThat(std::filesystem::exists(paths[0]), Is().False());
        AssertThat(std::filesystem::exists(paths[1]), Is().False());
        AssertThat(std::filesystem::exists(paths[2]), Is().False());
      });

      it("first deletes file on disk when reference count reaches 0 [scope]", []() {
        std::array<std::string, 2u + 1u> paths;
        {
          shared_levelized_file<int> lf1;
          lf1->touch();
          paths = lf1->paths();

          AssertThat(std::filesystem::exists(paths[0]), Is().True());
          AssertThat(std::filesystem::exists(paths[1]), Is().True());
          AssertThat(std::filesystem::exists(paths[2]), Is().True());
          {
            shared_levelized_file<int> lf2 = lf1;

            AssertThat(std::filesystem::exists(paths[0]), Is().True());
            AssertThat(std::filesystem::exists(paths[1]), Is().True());
            AssertThat(std::filesystem::exists(paths[2]), Is().True());
          }
          AssertThat(std::filesystem::exists(paths[0]), Is().True());
          AssertThat(std::filesystem::exists(paths[1]), Is().True());
          AssertThat(std::filesystem::exists(paths[2]), Is().True());
        }
        AssertThat(std::filesystem::exists(paths[0]), Is().False());
        AssertThat(std::filesystem::exists(paths[1]), Is().False());
        AssertThat(std::filesystem::exists(paths[2]), Is().False());
      });

      describe("w/ levelized_ofstream + levelized_ifstream + level_info_ifstream", []() {
        it("levelized_ofstream is part of reference counting", []() {
          std::array<std::string, 2u + 1u> paths;

          levelized_ofstream<int> lfw;
          {
            shared_levelized_file<int> lf;
            paths = lf->paths();
            lfw.open(lf);

            AssertThat(lf->exists(), Is().True());
            AssertThat(std::filesystem::exists(paths[0]), Is().True());
            AssertThat(std::filesystem::exists(paths[1]), Is().True());
            AssertThat(std::filesystem::exists(paths[2]), Is().True());
          }
          AssertThat(std::filesystem::exists(paths[0]), Is().True());
          AssertThat(std::filesystem::exists(paths[1]), Is().True());
          AssertThat(std::filesystem::exists(paths[2]), Is().True());

          lfw.close();

          AssertThat(std::filesystem::exists(paths[0]), Is().False());
          AssertThat(std::filesystem::exists(paths[1]), Is().False());
          AssertThat(std::filesystem::exists(paths[2]), Is().False());
        });

        it("levelized_ifstream is part of reference counting", []() {
          std::array<std::string, 2u + 1u> paths;

          levelized_ifstream<int> lfs;
          {
            shared_levelized_file<int> lf;
            paths = lf->paths();
            lfs.open(lf);

            AssertThat(lf->exists(), Is().True());
            AssertThat(std::filesystem::exists(paths[0]), Is().True());
            AssertThat(std::filesystem::exists(paths[1]), Is().True());
            AssertThat(std::filesystem::exists(paths[2]), Is().True());
          }
          AssertThat(std::filesystem::exists(paths[0]), Is().True());
          AssertThat(std::filesystem::exists(paths[1]), Is().True());
          AssertThat(std::filesystem::exists(paths[2]), Is().True());

          lfs.close();

          AssertThat(std::filesystem::exists(paths[0]), Is().False());
          AssertThat(std::filesystem::exists(paths[1]), Is().False());
          AssertThat(std::filesystem::exists(paths[2]), Is().False());
        });

        it("level_info_ifstream is part of reference counting", []() {
          std::array<std::string, 2u + 1u> paths;

          level_info_ifstream<> lis;
          {
            shared_levelized_file<int> lf;
            paths = lf->paths();
            lis.open(lf);

            AssertThat(lf->exists(), Is().True());
            AssertThat(std::filesystem::exists(paths[0]), Is().True());
            AssertThat(std::filesystem::exists(paths[1]), Is().True());
            AssertThat(std::filesystem::exists(paths[2]), Is().True());
          }
          AssertThat(std::filesystem::exists(paths[0]), Is().True());
          AssertThat(std::filesystem::exists(paths[1]), Is().True());
          AssertThat(std::filesystem::exists(paths[2]), Is().True());

          lis.close();

          AssertThat(std::filesystem::exists(paths[0]), Is().False());
          AssertThat(std::filesystem::exists(paths[1]), Is().False());
          AssertThat(std::filesystem::exists(paths[2]), Is().False());
        });

        it("can read written content", []() {
          shared_levelized_file<int> lf;
          {
            levelized_ofstream<int> lfw(lf);
            lfw.push<0>(42);
            lfw.push<0>(22);
            lfw.push<1>(1);
            lfw.push<0>(21);
            lfw.push<1>(2);
            lfw.push<1>(3);
            lfw.push(level_info{ 0, 2 });
            lfw.push(level_info{ 1, 3 });
            lfw.push(level_info{ 2, 1 });
          }
          {
            levelized_ifstream<int, false> lfs(lf); // <-- default: forwards
            level_info_ifstream<false> lis(lf);     // <-- default: backwards

            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(42));
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(22));
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(21));
            AssertThat(lfs.can_pull<0>(), Is().False());

            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(1));
            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(2));
            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(3));
            AssertThat(lfs.can_pull<1>(), Is().False());

            AssertThat(lis.can_pull(), Is().True());
            AssertThat(lis.pull(), Is().EqualTo(level_info{ 2, 1 }));
            AssertThat(lis.can_pull(), Is().True());
            AssertThat(lis.pull(), Is().EqualTo(level_info{ 1, 3 }));
            AssertThat(lis.can_pull(), Is().True());
            AssertThat(lis.pull(), Is().EqualTo(level_info{ 0, 2 }));
            AssertThat(lis.can_pull(), Is().False());
          }
        });

        it("can read written content in reverse", []() {
          shared_levelized_file<int> lf;
          {
            levelized_ofstream<int> lfw(lf);
            lfw.push<0>(16);
            lfw.push<0>(8);
            lfw.push<1>(2);
            lfw.push<0>(4);
            lfw.push<1>(4);
            lfw.push<1>(8);
            lfw.push<0>(1);
            lfw.push(level_info{ 0, 2 });
            lfw.push(level_info{ 1, 3 });
            lfw.push(level_info{ 2, 1 });
            lfw.push(level_info{ 4, 1 });
          }
          {
            levelized_ifstream<int, true> lfs(lf); // <-- default: forwards
            level_info_ifstream<true> lis(lf);     // <-- default: backwards

            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(1));
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(4));
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(8));
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(16));
            AssertThat(lfs.can_pull<0>(), Is().False());

            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(8));
            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(4));
            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(2));
            AssertThat(lfs.can_pull<1>(), Is().False());

            AssertThat(lis.can_pull(), Is().True());
            AssertThat(lis.pull(), Is().EqualTo(level_info{ 0, 2 }));
            AssertThat(lis.can_pull(), Is().True());
            AssertThat(lis.pull(), Is().EqualTo(level_info{ 1, 3 }));
            AssertThat(lis.can_pull(), Is().True());
            AssertThat(lis.pull(), Is().EqualTo(level_info{ 2, 1 }));
            AssertThat(lis.can_pull(), Is().True());
            AssertThat(lis.pull(), Is().EqualTo(level_info{ 4, 1 }));
            AssertThat(lis.can_pull(), Is().False());
          }
        });

        // TODO: file header content
      });

      describe("::copy(...)", []() {
        it("can copy over empty file", []() {
          shared_levelized_file<int> lf1;

          AssertThat(lf1->exists(), Is().False());
          lf1->touch();
          AssertThat(lf1->exists(), Is().True());
          AssertThat(lf1->empty(), Is().True());

          shared_levelized_file<int> lf2 = shared_levelized_file<int>::copy(lf1);

          AssertThat(lf2->exists(), Is().True());
          AssertThat(lf1.get(), Is().Not().EqualTo(lf2.get()));

          const auto paths1 = lf1->paths();
          const auto paths2 = lf2->paths();

          AssertThat(paths1[0], Is().Not().EqualTo(paths2[0]));
          AssertThat(paths1[0], Is().Not().EqualTo(paths2[1]));
          AssertThat(paths1[0], Is().Not().EqualTo(paths2[2]));

          AssertThat(paths1[1], Is().Not().EqualTo(paths2[0]));
          AssertThat(paths1[1], Is().Not().EqualTo(paths2[1]));
          AssertThat(paths1[1], Is().Not().EqualTo(paths2[2]));

          AssertThat(paths1[2], Is().Not().EqualTo(paths2[0]));
          AssertThat(paths1[2], Is().Not().EqualTo(paths2[1]));
          AssertThat(paths1[2], Is().Not().EqualTo(paths2[2]));
        });

        it("can copy over non-empty file", []() {
          shared_levelized_file<int> lf1;
          {
            levelized_ofstream<int> lfw(lf1);
            lfw.push<0>(2);
            lfw.push<0>(3);

            lfw.push(level_info{ 0, 1 });
          }

          shared_levelized_file<int> lf2 = shared_levelized_file<int>::copy(lf1);

          // Check paths
          const auto paths1 = lf1->paths();
          const auto paths2 = lf2->paths();

          AssertThat(paths1[0], Is().Not().EqualTo(paths2[0]));
          AssertThat(paths1[0], Is().Not().EqualTo(paths2[1]));
          AssertThat(paths1[0], Is().Not().EqualTo(paths2[2]));

          AssertThat(paths1[1], Is().Not().EqualTo(paths2[0]));
          AssertThat(paths1[1], Is().Not().EqualTo(paths2[1]));
          AssertThat(paths1[1], Is().Not().EqualTo(paths2[2]));

          AssertThat(paths1[2], Is().Not().EqualTo(paths2[0]));
          AssertThat(paths1[2], Is().Not().EqualTo(paths2[1]));
          AssertThat(paths1[2], Is().Not().EqualTo(paths2[2]));

          // Check content
          {
            levelized_ifstream<int> lfs(lf2);

            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(2));
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(3));
            AssertThat(lfs.can_pull<0>(), Is().False());

            AssertThat(lfs.can_pull<1>(), Is().False());

            level_info_ifstream<> lis(lf2);

            AssertThat(lis.can_pull(), Is().True());
            AssertThat(lis.pull(), Is().EqualTo(level_info{ 0, 1 }));
            AssertThat(lis.can_pull(), Is().False());
          }
        });
      });

      it("can move, persist and reopen a levelized file [/tmp/]", [&tmp_path]() {
        std::string path_prefix = tmp_path + "persisted-shared-path-prefix.adiar";

        // Clean up after prior test run
        if (std::filesystem::exists(path_prefix + ".file_0"))
          std::filesystem::remove(path_prefix + ".file_0");
        if (std::filesystem::exists(path_prefix + ".file_1"))
          std::filesystem::remove(path_prefix + ".file_1");
        if (std::filesystem::exists(path_prefix + ".levels"))
          std::filesystem::remove(path_prefix + ".levels");

        { // Create a persisted file
          shared_levelized_file<int> lf;

          levelized_ofstream<int> lfw(lf);
          lfw.push<0>(42);
          lfw.push(level_info{ 0, 1 });

          // TODO: header file content
          lfw.close();

          lf->move(path_prefix);
          lf->make_persistent();
        }

        {
          shared_levelized_file<int> lf(path_prefix);

          levelized_ifstream<int> lfs(lf);

          AssertThat(lfs.can_pull<0>(), Is().True());
          AssertThat(lfs.pull<0>(), Is().EqualTo(42));
          AssertThat(lfs.can_pull<0>(), Is().False());

          AssertThat(lfs.can_pull<1>(), Is().False());

          level_info_ifstream<false> lis(lf);

          AssertThat(lis.can_pull(), Is().True());
          AssertThat(lis.pull(), Is().EqualTo(level_info{ 0, 1 }));
          AssertThat(lis.can_pull(), Is().False());

          // TODO: header file content
        }

        // Clean up of this test
        if (std::filesystem::exists(path_prefix + ".file_0"))
          std::filesystem::remove(path_prefix + ".file_0");
        if (std::filesystem::exists(path_prefix + ".file_1"))
          std::filesystem::remove(path_prefix + ".file_1");
        if (std::filesystem::exists(path_prefix + ".levels"))
          std::filesystem::remove(path_prefix + ".levels");
      });

      it("can move, persist and reopen a levelized file [./]", [&curr_path]() {
        std::string path_prefix = curr_path + "persisted-shared-path-prefix.adiar";

        // Clean up after prior test run
        if (std::filesystem::exists(path_prefix + ".file_0"))
          std::filesystem::remove(path_prefix + ".file_0");
        if (std::filesystem::exists(path_prefix + ".file_1"))
          std::filesystem::remove(path_prefix + ".file_1");
        if (std::filesystem::exists(path_prefix + ".levels"))
          std::filesystem::remove(path_prefix + ".levels");

        { // Create a persisted file
          shared_levelized_file<int> lf;

          levelized_ofstream<int> lfw(lf);
          lfw.push<0>(21);
          lfw.push(level_info{ 0, 1 });
          lfw.push<1>(7);
          lfw.push<1>(14);
          lfw.push(level_info{ 1, 2 });

          // TODO: header file content
          lfw.close();

          lf->move(path_prefix);
          lf->make_persistent();
        }

        {
          shared_levelized_file<int> lf(path_prefix);

          levelized_ifstream<int> lfs(lf);

          AssertThat(lfs.can_pull<0>(), Is().True());
          AssertThat(lfs.pull<0>(), Is().EqualTo(21));
          AssertThat(lfs.can_pull<0>(), Is().False());

          AssertThat(lfs.can_pull<1>(), Is().True());
          AssertThat(lfs.pull<1>(), Is().EqualTo(7));
          AssertThat(lfs.can_pull<1>(), Is().True());
          AssertThat(lfs.pull<1>(), Is().EqualTo(14));
          AssertThat(lfs.can_pull<1>(), Is().False());

          level_info_ifstream<false> lis(lf);

          AssertThat(lis.can_pull(), Is().True());
          AssertThat(lis.pull(), Is().EqualTo(level_info{ 1, 2 }));
          AssertThat(lis.can_pull(), Is().True());
          AssertThat(lis.pull(), Is().EqualTo(level_info{ 0, 1 }));
          AssertThat(lis.can_pull(), Is().False());

          // TODO: header file content
        }

        // Clean up of this test
        if (std::filesystem::exists(path_prefix + ".file_0"))
          std::filesystem::remove(path_prefix + ".file_0");
        if (std::filesystem::exists(path_prefix + ".file_1"))
          std::filesystem::remove(path_prefix + ".file_1");
        if (std::filesystem::exists(path_prefix + ".levels"))
          std::filesystem::remove(path_prefix + ".levels");
      });
    });
  });
});
