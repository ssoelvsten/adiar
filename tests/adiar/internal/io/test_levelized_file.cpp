#include "../../../test.h"
#include <array>
#include <filesystem>

go_bandit([]() {
  describe(
    "adiar/internal/io/levelized_file.h , levelized_ifstream.h , levelized_ofstream.h", []() {
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

      it("levelized_file::FILES exposes the file_traits",
         []() { AssertThat(levelized_file<int>::FILES, Is().EqualTo(2u)); });

      describe("levelized_file()", [&tmp_path, &curr_path]() {
        it("creates a file marked as 'temporary'", []() {
          levelized_file<int> lf;
          AssertThat(lf.is_persistent(), Is().False());
          AssertThat(lf.is_temp(), Is().True());
        });

        it("has no content", []() {
          levelized_file<int> lf;
          AssertThat(lf.size(), Is().EqualTo(0u));
          AssertThat(lf.empty(), Is().True());
        });

        it("has no levels", []() {
          levelized_file<int> lf;
          AssertThat(lf.levels(), Is().EqualTo(0u));
        });

        it("has non-canonical paths", []() {
          levelized_file<int> lf;
          AssertThat(lf.canonical_paths(), Is().False());
        });

        it("has FILES+1 different paths", []() {
          levelized_file<int> lf;
          auto paths = lf.paths();

          // Check array has expected size.
          AssertThat(paths.size(), Is().EqualTo(2u + 1u));

          // Check array has non-trivial values
          AssertThat(paths[0], Is().Not().EqualTo(""));
          AssertThat(paths[1], Is().Not().EqualTo(""));
          AssertThat(paths[2], Is().Not().EqualTo(""));

          // Check all are different
          AssertThat(paths[0], Is().Not().EqualTo(paths[1]));
          AssertThat(paths[0], Is().Not().EqualTo(paths[2]));
          AssertThat(paths[1], Is().Not().EqualTo(paths[2]));
        });

        it("does not (yet) 'exist' on disk", []() {
          levelized_file<int> lf;
          AssertThat(lf.exists(), Is().False());

          auto paths = lf.paths();
          AssertThat(std::filesystem::exists(paths[0]), Is().False());
          AssertThat(std::filesystem::exists(paths[1]), Is().False());
          AssertThat(std::filesystem::exists(paths[2]), Is().False());
        });

        it("'exists' after a 'touch'", []() {
          levelized_file<int> lf;
          AssertThat(lf.exists(), Is().False());

          auto paths = lf.paths();

          lf.touch();
          AssertThat(lf.exists(), Is().True());

          AssertThat(std::filesystem::exists(paths[0]), Is().True());
          AssertThat(std::filesystem::exists(paths[1]), Is().True());
          AssertThat(std::filesystem::exists(paths[2]), Is().True());
        });

        it("is a fresh file", []() {
          levelized_file<int> lf1;
          auto paths1 = lf1.paths();

          levelized_file<int> lf2;
          auto paths2 = lf2.paths();

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

        it("is temporary (removed from disk after destruct)", []() {
          std::array<std::string, 2u + 1u> paths;
          {
            levelized_file<int> lf;
            paths = lf.paths();
            lf.touch();

            AssertThat(std::filesystem::exists(paths[0]), Is().True());
            AssertThat(std::filesystem::exists(paths[1]), Is().True());
            AssertThat(std::filesystem::exists(paths[2]), Is().True());
          }
          AssertThat(std::filesystem::exists(paths[0]), Is().False());
          AssertThat(std::filesystem::exists(paths[1]), Is().False());
          AssertThat(std::filesystem::exists(paths[2]), Is().False());
        });

        it("can be 'moved' when not existing [/tmp/]", [&tmp_path]() {
          const std::string path_prefix = tmp_path + "after-move-path.adiar";

          // Clean up after prior test run
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          levelized_file<int> lf;
          std::array<std::string, 2u + 1u> old_paths = lf.paths();

          AssertThat(std::filesystem::exists(old_paths[0]), Is().False());
          AssertThat(std::filesystem::exists(old_paths[1]), Is().False());
          AssertThat(std::filesystem::exists(old_paths[2]), Is().False());

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().False());

          lf.move(path_prefix);

          AssertThat(std::filesystem::exists(old_paths[0]), Is().False());
          AssertThat(std::filesystem::exists(old_paths[1]), Is().False());
          AssertThat(std::filesystem::exists(old_paths[2]), Is().False());

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().False());

          lf.touch();

          AssertThat(std::filesystem::exists(old_paths[0]), Is().False());
          AssertThat(std::filesystem::exists(old_paths[1]), Is().False());
          AssertThat(std::filesystem::exists(old_paths[2]), Is().False());

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().True());
        });

        it("can be 'moved' when not existing [./]", [&curr_path]() {
          const std::string path_prefix = curr_path + "after-move-path.adiar";

          // Clean up after prior test run
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          levelized_file<int> lf;
          std::array<std::string, 2u + 1u> old_paths = lf.paths();

          AssertThat(std::filesystem::exists(old_paths[0]), Is().False());
          AssertThat(std::filesystem::exists(old_paths[1]), Is().False());
          AssertThat(std::filesystem::exists(old_paths[2]), Is().False());

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().False());

          lf.move(path_prefix);

          AssertThat(std::filesystem::exists(old_paths[0]), Is().False());
          AssertThat(std::filesystem::exists(old_paths[1]), Is().False());
          AssertThat(std::filesystem::exists(old_paths[2]), Is().False());

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().False());

          lf.touch();

          AssertThat(std::filesystem::exists(old_paths[0]), Is().False());
          AssertThat(std::filesystem::exists(old_paths[1]), Is().False());
          AssertThat(std::filesystem::exists(old_paths[2]), Is().False());

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().True());
        });

        it("has canonical paths after 'move' [/tmp/]", [&tmp_path]() {
          const std::string path_prefix = tmp_path + "after-move-path.adiar";

          // Clean up after prior test run
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          levelized_file<int> lf;
          std::array<std::string, 2u + 1u> old_paths = lf.paths();

          AssertThat(lf.canonical_paths(), Is().False());
          lf.move(path_prefix);
          AssertThat(lf.canonical_paths(), Is().True());
        });

        it("can be 'moved' when existing [/tmp/]", [&tmp_path]() {
          const std::string path_prefix = tmp_path + "after-move-path.adiar";

          // Clean up after prior test run
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          levelized_file<int> lf;
          lf.touch();
          std::array<std::string, 2u + 1u> old_paths = lf.paths();

          AssertThat(std::filesystem::exists(old_paths[0]), Is().True());
          AssertThat(std::filesystem::exists(old_paths[1]), Is().True());
          AssertThat(std::filesystem::exists(old_paths[2]), Is().True());

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().False());

          lf.move(path_prefix);

          AssertThat(std::filesystem::exists(old_paths[0]), Is().False());
          AssertThat(std::filesystem::exists(old_paths[1]), Is().False());
          AssertThat(std::filesystem::exists(old_paths[2]), Is().False());

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().True());
        });

        it("can be 'moved' when existing [./]", [&curr_path]() {
          const std::string path_prefix = curr_path + "after-move-path.adiar";

          // Clean up after prior test run
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          levelized_file<int> lf;
          lf.touch();
          std::array<std::string, 2u + 1u> old_paths = lf.paths();

          AssertThat(std::filesystem::exists(old_paths[0]), Is().True());
          AssertThat(std::filesystem::exists(old_paths[1]), Is().True());
          AssertThat(std::filesystem::exists(old_paths[2]), Is().True());

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().False());

          lf.move(path_prefix);

          AssertThat(std::filesystem::exists(old_paths[0]), Is().False());
          AssertThat(std::filesystem::exists(old_paths[1]), Is().False());
          AssertThat(std::filesystem::exists(old_paths[2]), Is().False());

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().True());
        });

        it("is still temporary after move [/tmp/]", [&tmp_path]() {
          const std::string path_prefix = tmp_path + "after-move-path.adiar";

          // Clean up after prior test run
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          {
            levelized_file<int> lf;
            lf.touch();
            lf.move(path_prefix);

            AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().True());
            AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().True());
            AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().True());

            AssertThat(lf.is_persistent(), Is().False());
            AssertThat(lf.is_temp(), Is().True());
          }

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().False());
        });

        it("is still temporary after move [./]", [&curr_path]() {
          const std::string path_prefix = curr_path + "after-move-path.adiar";

          // Clean up after prior test run
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          {
            levelized_file<int> lf;
            lf.touch();
            lf.move(path_prefix);

            AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().True());
            AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().True());
            AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().True());

            AssertThat(lf.is_persistent(), Is().False());
            AssertThat(lf.is_temp(), Is().True());
          }

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().False());
        });

        it("cannot be 'moved' on-top of the path prefix itself [/tmp/]", [&tmp_path]() {
          const std::string path_prefix = tmp_path + "after-move-path.adiar";

          // Clean up after prior test run
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          {
            file<int> f;
            f.move(path_prefix);
            f.touch();

            levelized_file<int> lf;

            AssertThrows(runtime_error, lf.move(path_prefix));
          }

          // Clean up of this test
          if (std::filesystem::exists(path_prefix)) std::filesystem::remove(path_prefix);
        });

        it("cannot be 'moved' on-top of another existing underlying file [/tmp/]", [&tmp_path]() {
          const std::string path_prefix = tmp_path + "after-move-path.adiar";

          // Clean up after prior test run
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          levelized_file<int> lf1;
          lf1.move(path_prefix);
          lf1.touch();

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().True());

          levelized_file<int> lf2;

          AssertThrows(runtime_error, lf2.move(path_prefix));
        });

        it("can be made persistent (not removed from disk)", [&curr_path]() {
          const std::string path_prefix = curr_path + "after-move-path.adiar";

          // Clean up after prior test run
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          {
            levelized_file<int> lf;
            std::array<std::string, 2u + 1u> paths = lf.paths();
            lf.touch();

            AssertThat(std::filesystem::exists(paths[0]), Is().True());
            AssertThat(std::filesystem::exists(paths[1]), Is().True());
            AssertThat(std::filesystem::exists(paths[2]), Is().True());

            lf.make_persistent(path_prefix);

            AssertThat(std::filesystem::exists(paths[0]), Is().False());
            AssertThat(std::filesystem::exists(paths[1]), Is().False());
            AssertThat(std::filesystem::exists(paths[2]), Is().False());
          }

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().True());

          // Clean up for this test
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");
        });

        it("exists on disk after being made persistent", [&tmp_path]() {
          const std::string path_prefix = tmp_path + "after-move-path.adiar";

          // Clean up after prior test run
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          {
            levelized_file<int> lf;
            std::array<std::string, 2u + 1u> paths = lf.paths();

            AssertThat(std::filesystem::exists(paths[0]), Is().False());
            AssertThat(std::filesystem::exists(paths[1]), Is().False());
            AssertThat(std::filesystem::exists(paths[2]), Is().False());

            lf.make_persistent(path_prefix);

            AssertThat(std::filesystem::exists(paths[0]), Is().False());
            AssertThat(std::filesystem::exists(paths[1]), Is().False());
            AssertThat(std::filesystem::exists(paths[2]), Is().False());
          }

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().True());

          // Clean up for this test
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");
        });

        it("cannot 'move' file marked persisted [/tmp/]", [&tmp_path]() {
          const std::string path_prefix = tmp_path + "after-move-path.adiar";

          // Clean up after prior test run
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          {
            levelized_file<int> lf;
            lf.make_persistent(path_prefix);
            AssertThrows(runtime_error, lf.move(path_prefix));
          }

          // Clean up for this test
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");
        });
      });

      describe("levelized_file() + levelized_ifstream", []() {
        it("it can attach to and detach from an empty file [con-/destructor]", []() {
          levelized_file<int> lf;
          levelized_ifstream<int> lfs(lf);
        });

        it("it can attach to and detach from an empty file [member functions]", []() {
          levelized_file<int> lf;
          levelized_ifstream<int> lfs;
          lfs.open(lf);
          lfs.close();
        });

        it("it remembers it was attached", []() {
          levelized_file<int> lf;
          levelized_ifstream<int> lfs(lf);

          AssertThat(lfs.is_open(), Is().True());
          lfs.close();
          AssertThat(lfs.is_open(), Is().False());
        });

        it("it cannot be pulled from", []() {
          levelized_file<int> lf;
          levelized_ifstream<int> lfs(lf);

          AssertThat(lfs.template can_pull<0>(), Is().False());
          AssertThat(lfs.template can_pull<1>(), Is().False());
        });

        it("it can be reset", []() {
          levelized_file<int> lf;
          levelized_ifstream<int> lfs(lf);

          lfs.reset();
          AssertThat(lfs.is_open(), Is().True());
          AssertThat(lfs.template can_pull<0>(), Is().False());
          AssertThat(lfs.template can_pull<1>(), Is().False());
        });
      });

      describe("levelized_file() + levelized_ofstream", [&tmp_path, &curr_path]() {
        it("it can attach to and detach from an empty file [con-/destructor]", []() {
          levelized_file<int> lf;
          levelized_ofstream<int> lfw(lf);
        });

        it("it can attach to and detach from an empty file [member functions]", []() {
          levelized_file<int> lf;
          levelized_ofstream<int> lfw;
          lfw.open(lf);
          lfw.close();
        });

        it("it remembers it was attached", []() {
          levelized_file<int> lf;
          levelized_ofstream<int> lfw(lf);

          AssertThat(lfw.is_open(), Is().True());
          lfw.close();
          AssertThat(lfw.is_open(), Is().False());
        });

        it("exists after writer attach", []() {
          levelized_file<int> lf;
          AssertThat(lf.exists(), Is().False());

          levelized_ofstream<int> lfw(lf);
          lfw.close();

          AssertThat(lf.exists(), Is().True());
        });

        it("reports whether elements were pushed [int<0>]", []() {
          levelized_file<int> lf;

          levelized_ofstream<int> lfw(lf);

          AssertThat(lfw.has_pushed(), Is().False());
          AssertThat(lfw.empty(), Is().True());

          lfw.push<0>(42);

          AssertThat(lfw.has_pushed(), Is().True());
          AssertThat(lfw.empty(), Is().False());
        });

        it("reports whether elements were pushed [int<1>]", []() {
          levelized_file<int> lf;

          levelized_ofstream<int> lfw(lf);

          AssertThat(lfw.has_pushed(), Is().False());
          AssertThat(lfw.empty(), Is().True());

          lfw.push<1>(42);

          AssertThat(lfw.has_pushed(), Is().True());
          AssertThat(lfw.empty(), Is().False());
        });

        it("reports whether elements were pushed [level_info]", []() {
          levelized_file<int> lf;

          levelized_ofstream<int> lfw(lf);

          AssertThat(lfw.has_pushed(), Is().False());
          AssertThat(lfw.empty(), Is().True());

          lfw << level_info{ 0, 2 };

          AssertThat(lfw.has_pushed(), Is().True());
          AssertThat(lfw.empty(), Is().False());
        });

        it("changes size when writing content to '.file_0'", []() {
          levelized_file<int> lf;
          levelized_ofstream<int> lfw(lf);

          AssertThat(lfw.size(), Is().EqualTo(0u));
          AssertThat(lfw.size(0), Is().EqualTo(0u));
          AssertThat(lfw.size(1), Is().EqualTo(0u));
          AssertThat(lfw.levels(), Is().EqualTo(0u));

          lfw.push<0>(42);

          AssertThat(lfw.size(), Is().EqualTo(1u));
          AssertThat(lfw.size(0), Is().EqualTo(1u));
          AssertThat(lfw.size(1), Is().EqualTo(0u));
          AssertThat(lfw.levels(), Is().EqualTo(0u));

          lfw.push<0>(21);

          AssertThat(lfw.size(), Is().EqualTo(2u));
          AssertThat(lfw.size(0), Is().EqualTo(2u));
          AssertThat(lfw.size(1), Is().EqualTo(0u));
          AssertThat(lfw.levels(), Is().EqualTo(0u));

          lfw.close();

          AssertThat(lf.size(), Is().EqualTo(2u));
          AssertThat(lf.size(0), Is().EqualTo(2u));
          AssertThat(lf.size(1), Is().EqualTo(0u));
          AssertThat(lf.levels(), Is().EqualTo(0u));
        });

        it("changes size when writing content to '.file_1'", []() {
          levelized_file<int> lf;
          levelized_ofstream<int> lfw(lf);

          AssertThat(lfw.size(), Is().EqualTo(0u));
          AssertThat(lfw.size(0), Is().EqualTo(0u));
          AssertThat(lfw.size(1), Is().EqualTo(0u));

          lfw.push<1>(42);

          AssertThat(lfw.size(), Is().EqualTo(1u));
          AssertThat(lfw.size(0), Is().EqualTo(0u));
          AssertThat(lfw.size(1), Is().EqualTo(1u));

          lfw.push<1>(21);

          AssertThat(lfw.size(), Is().EqualTo(2u));
          AssertThat(lfw.size(0), Is().EqualTo(0u));
          AssertThat(lfw.size(1), Is().EqualTo(2u));

          lfw.close();

          AssertThat(lf.size(), Is().EqualTo(2u));
          AssertThat(lf.size(0), Is().EqualTo(0u));
          AssertThat(lf.size(1), Is().EqualTo(2u));
          AssertThat(lf.levels(), Is().EqualTo(0u));
        });

        it("changes size when writing content to both '.file_0' and '.file_1'", []() {
          levelized_file<int> lf;
          levelized_ofstream<int> lfw(lf);

          AssertThat(lfw.size(), Is().EqualTo(0u));
          AssertThat(lfw.size(0), Is().EqualTo(0u));
          AssertThat(lfw.size(1), Is().EqualTo(0u));
          AssertThat(lfw.levels(), Is().EqualTo(0u));

          lfw.push<0>(5);

          AssertThat(lfw.size(), Is().EqualTo(1u));
          AssertThat(lfw.size(0), Is().EqualTo(1u));
          AssertThat(lfw.size(1), Is().EqualTo(0u));
          AssertThat(lfw.levels(), Is().EqualTo(0u));

          lfw.push<1>(2);

          AssertThat(lfw.size(), Is().EqualTo(2u));
          AssertThat(lfw.size(0), Is().EqualTo(1u));
          AssertThat(lfw.size(1), Is().EqualTo(1u));
          AssertThat(lfw.levels(), Is().EqualTo(0u));

          lfw.push<1>(4);

          AssertThat(lfw.size(), Is().EqualTo(3u));
          AssertThat(lfw.size(0), Is().EqualTo(1u));
          AssertThat(lfw.size(1), Is().EqualTo(2u));
          AssertThat(lfw.levels(), Is().EqualTo(0u));

          lfw.push<0>(3);

          AssertThat(lfw.size(), Is().EqualTo(4u));
          AssertThat(lfw.size(0), Is().EqualTo(2u));
          AssertThat(lfw.size(1), Is().EqualTo(2u));
          AssertThat(lfw.levels(), Is().EqualTo(0u));

          lfw.push<1>(8);

          AssertThat(lfw.size(), Is().EqualTo(5u));
          AssertThat(lfw.size(0), Is().EqualTo(2u));
          AssertThat(lfw.size(1), Is().EqualTo(3u));
          AssertThat(lfw.levels(), Is().EqualTo(0u));

          lfw.close();

          AssertThat(lf.size(), Is().EqualTo(5u));
          AssertThat(lf.size(0), Is().EqualTo(2u));
          AssertThat(lf.size(1), Is().EqualTo(3u));
          AssertThat(lf.levels(), Is().EqualTo(0u));
        });

        it("changes size when writing content to '.levels'", []() {
          levelized_file<int> lf;
          levelized_ofstream<int> lfw(lf);

          AssertThat(lfw.size(), Is().EqualTo(0u));
          AssertThat(lfw.levels(), Is().EqualTo(0u));

          lfw.push(level_info{ 0, 3 });

          AssertThat(lfw.size(), Is().EqualTo(0u));
          AssertThat(lfw.levels(), Is().EqualTo(1u));

          lfw.push(level_info{ 1, 2 });

          AssertThat(lfw.size(), Is().EqualTo(0u));
          AssertThat(lfw.levels(), Is().EqualTo(2u));

          lfw.push(level_info{ 2, 1 });

          AssertThat(lfw.size(), Is().EqualTo(0u));
          AssertThat(lfw.levels(), Is().EqualTo(3u));

          lfw.close();

          AssertThat(lf.size(), Is().EqualTo(0u));
          AssertThat(lf.levels(), Is().EqualTo(3u));
        });

        it("can be 'moved' after write [/tmp/]", [&tmp_path]() {
          const std::string path_prefix = tmp_path + "after-move-prefix.adiar";

          // Clean up after prior tests
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          levelized_file<int> lf;
          levelized_ofstream<int> lfw(lf);
          lfw.push<0>(42);
          lfw.push<1>(2);
          lfw.push<0>(21);
          lfw.push<1>(3);
          lfw.push<1>(1);
          lfw.push(level_info{ 0, 2 });
          lfw.push(level_info{ 2, 1 });
          lfw.push(level_info{ 3, 2 });
          lfw.close();

          std::array<std::string, 2u + 1u> paths = lf.paths();

          AssertThat(std::filesystem::exists(paths[0]), Is().True());
          AssertThat(std::filesystem::exists(paths[1]), Is().True());
          AssertThat(std::filesystem::exists(paths[2]), Is().True());

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().False());

          lf.move(path_prefix);

          AssertThat(std::filesystem::exists(paths[0]), Is().False());
          AssertThat(std::filesystem::exists(paths[1]), Is().False());
          AssertThat(std::filesystem::exists(paths[2]), Is().False());

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().True());
        });

        it("can be 'moved' after write [./]", [&curr_path]() {
          const std::string path_prefix = curr_path + "after-move-prefix.adiar";

          // Clean up after prior tests
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          levelized_file<int> lf;
          levelized_ofstream<int> lfw(lf);
          lfw.push<1>(42);
          lfw.push<0>(2);
          lfw.push<0>(21);
          lfw.push<0>(3);
          lfw.push(level_info{ 0, 1 });
          lfw.push(level_info{ 2, 1 });
          lfw.push(level_info{ 3, 2 });
          lfw.close();

          std::array<std::string, 2u + 1u> paths = lf.paths();

          AssertThat(std::filesystem::exists(paths[0]), Is().True());
          AssertThat(std::filesystem::exists(paths[1]), Is().True());
          AssertThat(std::filesystem::exists(paths[2]), Is().True());

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().False());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().False());

          lf.move(path_prefix);

          AssertThat(std::filesystem::exists(paths[0]), Is().False());
          AssertThat(std::filesystem::exists(paths[1]), Is().False());
          AssertThat(std::filesystem::exists(paths[2]), Is().False());

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().True());
        });
      });

      describe("levelized_file() + levelized_ifstream + levelized_ofstream",
               [&tmp_path /*, &curr_path*/]() {
                 it("can read written content", []() {
                   levelized_file<int> lf;
                   {
                     levelized_ofstream<int> lfw(lf);
                     lfw.push<0>(23);
                     lfw.push<0>(8);
                     lfw.push<1>(16);
                     lfw.push<0>(32);
                     lfw.push<1>(24);
                   }
                   {
                     levelized_ifstream<int> lfs(lf);
                     AssertThat(lfs.pull<0>(), Is().EqualTo(23));
                     AssertThat(lfs.pull<0>(), Is().EqualTo(8));
                     AssertThat(lfs.pull<0>(), Is().EqualTo(32));
                     AssertThat(lfs.pull<1>(), Is().EqualTo(16));
                     AssertThat(lfs.pull<1>(), Is().EqualTo(24));
                   }
                 });

                 it("can read written content in reverse", []() {
                   levelized_file<int> lf;
                   {
                     levelized_ofstream<int> lfw(lf);
                     lfw.push<0>(18);
                     lfw.push<1>(24);
                     lfw.push<0>(32);
                     lfw.push<1>(21);
                     lfw.push<1>(23);
                     lfw.push<0>(16);
                   }
                   {
                     levelized_ifstream<int, true> lfs(lf);
                     AssertThat(lfs.pull<0>(), Is().EqualTo(16));
                     AssertThat(lfs.pull<0>(), Is().EqualTo(32));
                     AssertThat(lfs.pull<0>(), Is().EqualTo(18));
                     AssertThat(lfs.pull<1>(), Is().EqualTo(23));
                     AssertThat(lfs.pull<1>(), Is().EqualTo(21));
                     AssertThat(lfs.pull<1>(), Is().EqualTo(24));
                   }
                 });

                 it("can read written content after move", [&tmp_path]() {
                   const std::string path_prefix = tmp_path + "after-move-prefix.adiar";

                   // Clean up after prior tests
                   if (std::filesystem::exists(path_prefix + ".file_0"))
                     std::filesystem::remove(path_prefix + ".file_0");
                   if (std::filesystem::exists(path_prefix + ".file_1"))
                     std::filesystem::remove(path_prefix + ".file_1");
                   if (std::filesystem::exists(path_prefix + ".levels"))
                     std::filesystem::remove(path_prefix + ".levels");

                   levelized_file<int> lf;
                   {
                     levelized_ofstream<int> lfw(lf);
                     lfw.push<0>(23);
                     lfw.push<0>(32);
                     lfw.push<1>(24);
                   }
                   lf.move(path_prefix);
                   {
                     levelized_ifstream<int> lfs(lf);
                     AssertThat(lfs.pull<0>(), Is().EqualTo(23));
                     AssertThat(lfs.pull<0>(), Is().EqualTo(32));
                     AssertThat(lfs.pull<1>(), Is().EqualTo(24));
                   }
                 });

                 it("can read written content in reverse after move", [&tmp_path]() {
                   const std::string path_prefix = tmp_path + "after-move-prefix.adiar";

                   // Clean up after prior tests
                   if (std::filesystem::exists(path_prefix + ".file_0"))
                     std::filesystem::remove(path_prefix + ".file_0");
                   if (std::filesystem::exists(path_prefix + ".file_1"))
                     std::filesystem::remove(path_prefix + ".file_1");
                   if (std::filesystem::exists(path_prefix + ".levels"))
                     std::filesystem::remove(path_prefix + ".levels");

                   levelized_file<int> lf;
                   {
                     levelized_ofstream<int> lfw(lf);
                     lfw.push<0>(0);
                     lfw.push<1>(1);
                     lfw.push<0>(2);
                     lfw.push<1>(3);
                     lfw.push<0>(4);
                     lfw.push<1>(5);
                     lfw.push<0>(6);
                     lfw.push<1>(7);
                   }
                   lf.move(path_prefix);
                   {
                     levelized_ifstream<int, true> lfs(lf);
                     AssertThat(lfs.pull<0>(), Is().EqualTo(6));
                     AssertThat(lfs.pull<0>(), Is().EqualTo(4));
                     AssertThat(lfs.pull<0>(), Is().EqualTo(2));
                     AssertThat(lfs.pull<0>(), Is().EqualTo(0));
                     AssertThat(lfs.pull<1>(), Is().EqualTo(7));
                     AssertThat(lfs.pull<1>(), Is().EqualTo(5));
                     AssertThat(lfs.pull<1>(), Is().EqualTo(3));
                     AssertThat(lfs.pull<1>(), Is().EqualTo(1));
                   }
                 });
               });

      describe("levelized_file() + levelized_ofstream + level_info_ifstream", []() {
        levelized_file<int> lf;
        levelized_ofstream lfw(lf);

        lfw.push<0>(-1);
        lfw.push<0>(1);
        lfw.push<1>(-1);
        lfw.push<0>(2);
        lfw.push<1>(1);
        lfw.push(level_info{ 1u, 2u });
        lfw.push(level_info{ 3u, 3u });

        lfw.close();

        it("pulls by default level information in reverse", [&]() {
          level_info_ifstream fs(lf);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(level_info{ 3u, 3u }));

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(level_info{ 1u, 2u }));

          AssertThat(fs.can_pull(), Is().False());
        });

        it("peeks by default level information in reverse", [&]() {
          level_info_ifstream fs(lf);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(level_info{ 3u, 3u }));
          AssertThat(fs.pull(), Is().EqualTo(level_info{ 3u, 3u }));

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(level_info{ 1u, 2u }));
          AssertThat(fs.peek(), Is().EqualTo(level_info{ 1u, 2u }));
          AssertThat(fs.pull(), Is().EqualTo(level_info{ 1u, 2u }));

          AssertThat(fs.can_pull(), Is().False());
        });

        it("can read level information forwards", [&]() {
          level_info_ifstream<true> fs(lf);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(level_info{ 1u, 2u }));

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.pull(), Is().EqualTo(level_info{ 3u, 3u }));

          AssertThat(fs.can_pull(), Is().False());
        });

        it("can peek level information forwards", [&]() {
          level_info_ifstream<true> fs(lf);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(level_info{ 1u, 2u }));
          AssertThat(fs.peek(), Is().EqualTo(level_info{ 1u, 2u }));
          AssertThat(fs.pull(), Is().EqualTo(level_info{ 1u, 2u }));

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(level_info{ 3u, 3u }));
          AssertThat(fs.pull(), Is().EqualTo(level_info{ 3u, 3u }));

          AssertThat(fs.can_pull(), Is().False());
        });

        it("shifts level information [+0]", [&]() {
          level_info_ifstream fs(lf, +0);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(level_info{ 3u, 3u }));
          AssertThat(fs.pull(), Is().EqualTo(level_info{ 3u, 3u }));

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(level_info{ 1u, 2u }));
          AssertThat(fs.pull(), Is().EqualTo(level_info{ 1u, 2u }));

          AssertThat(fs.can_pull(), Is().False());
        });

        it("shifts level information [+1]", [&]() {
          level_info_ifstream fs(lf, +1);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(level_info{ 4u, 3u }));
          AssertThat(fs.pull(), Is().EqualTo(level_info{ 4u, 3u }));

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(level_info{ 2u, 2u }));
          AssertThat(fs.pull(), Is().EqualTo(level_info{ 2u, 2u }));

          AssertThat(fs.can_pull(), Is().False());
        });

        it("shifts level information [+2]", [&]() {
          level_info_ifstream fs(lf, +2);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(level_info{ 5u, 3u }));
          AssertThat(fs.pull(), Is().EqualTo(level_info{ 5u, 3u }));

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(level_info{ 3u, 2u }));
          AssertThat(fs.pull(), Is().EqualTo(level_info{ 3u, 2u }));

          AssertThat(fs.can_pull(), Is().False());
        });

        it("shifts level information [-1]", [&]() {
          level_info_ifstream fs(lf, -1);

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(level_info{ 2u, 3u }));
          AssertThat(fs.pull(), Is().EqualTo(level_info{ 2u, 3u }));

          AssertThat(fs.can_pull(), Is().True());
          AssertThat(fs.peek(), Is().EqualTo(level_info{ 0u, 2u }));
          AssertThat(fs.pull(), Is().EqualTo(level_info{ 0u, 2u }));

          AssertThat(fs.can_pull(), Is().False());
        });
      });

      describe("levelized_file(path_prefix)", [&tmp_path, &curr_path]() {
        it("throws exception on path-prefix to completely non-existing files", [&curr_path]() {
          const std::string path_prefix = curr_path + "non-existing-prefix.adiar";
          AssertThrows(runtime_error, levelized_file<int>(path_prefix));
        });

        it("throws exception on path-prefix to partially non-existing files [missing .file_0]",
           [&curr_path]() {
             const std::string path_prefix = curr_path + "non-existing-prefix.adiar";
             { // Create the 'existing' files by hand (and clean up after prior tests)
               // TODO: take the respective file headers into account
               if (std::filesystem::exists(path_prefix + ".file_0"))
                 std::filesystem::remove(path_prefix + ".file_0");

               if (std::filesystem::exists(path_prefix + ".file_1"))
                 std::filesystem::remove(path_prefix + ".file_1");

               file<int> f_1;
               f_1.move(path_prefix + ".file_1");
               f_1.touch();
               f_1.make_persistent();

               if (std::filesystem::exists(path_prefix + ".levels"))
                 std::filesystem::remove(path_prefix + ".levels");

               file<level_info> f_levels;
               f_levels.move(path_prefix + ".levels");
               f_levels.touch();
               f_levels.make_persistent();
             }

             AssertThrows(runtime_error, levelized_file<int>(path_prefix));

             // Cleanup after this test
             if (std::filesystem::exists(path_prefix + ".file_1"))
               std::filesystem::remove(path_prefix + ".file_1");
             if (std::filesystem::exists(path_prefix + ".levels"))
               std::filesystem::remove(path_prefix + ".levels");
           });

        it("throws exception on path-prefix to partially non-existing files [missing .file_1]",
           [&curr_path]() {
             const std::string path_prefix = curr_path + "non-existing-prefix.adiar";
             { // Create the 'existing' files by hand (and clean up after prior tests)
               // TODO: take the respective file headers into account
               if (std::filesystem::exists(path_prefix + ".file_0"))
                 std::filesystem::remove(path_prefix + ".file_0");

               file<int> f_0;
               f_0.move(path_prefix + ".file_0");
               f_0.touch();
               f_0.make_persistent();

               if (std::filesystem::exists(path_prefix + ".file_1"))
                 std::filesystem::remove(path_prefix + ".file_1");

               if (std::filesystem::exists(path_prefix + ".levels"))
                 std::filesystem::remove(path_prefix + ".levels");

               file<level_info> f_levels;
               f_levels.move(path_prefix + ".levels");
               f_levels.touch();
               f_levels.make_persistent();
             }

             AssertThrows(runtime_error, levelized_file<int>(path_prefix));

             // Cleanup after this test
             if (std::filesystem::exists(path_prefix + ".file_0"))
               std::filesystem::remove(path_prefix + ".file_0");
             if (std::filesystem::exists(path_prefix + ".levels"))
               std::filesystem::remove(path_prefix + ".levels");
           });

        it("throws exception on path-prefix to partially non-existing files [missing .levels]",
           [&curr_path]() {
             const std::string path_prefix = curr_path + "non-existing-prefix.adiar";
             { // Create the 'existing' files by hand (and clean up after prior tests)
               // TODO: take the respective file headers into account
               if (std::filesystem::exists(path_prefix + ".file_0"))
                 std::filesystem::remove(path_prefix + ".file_0");

               file<int> f_0;
               f_0.move(path_prefix + ".file_0");
               f_0.touch();
               f_0.make_persistent();

               if (std::filesystem::exists(path_prefix + ".file_1"))
                 std::filesystem::remove(path_prefix + ".file_1");

               file<int> f_1;
               f_1.move(path_prefix + ".file_1");
               f_1.touch();
               f_1.make_persistent();

               if (std::filesystem::exists(path_prefix + ".levels"))
                 std::filesystem::remove(path_prefix + ".levels");
             }

             AssertThrows(runtime_error, levelized_file<int>(path_prefix));

             // Cleanup after this test
             if (std::filesystem::exists(path_prefix + ".file_0"))
               std::filesystem::remove(path_prefix + ".file_0");
             if (std::filesystem::exists(path_prefix + ".file_1"))
               std::filesystem::remove(path_prefix + ".file_1");
           });

        it("can reopen a persisted empty file", [&tmp_path]() {
          const std::string path_prefix = tmp_path + "existing-prefix.adiar";

          // Clean up after prior tests
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          {
            levelized_file<int> lf;
            lf.make_persistent(path_prefix);
          }
          {
            levelized_file<int> lf(path_prefix);
          }

          // Cleanup after this test
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");
        });

        it("has a persisted empty file still marked persistnt", [&tmp_path]() {
          const std::string path_prefix = tmp_path + "existing-prefix.adiar";

          // Clean up after prior tests
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          {
            levelized_file<int> lf;
            lf.make_persistent(path_prefix);
          }
          {
            levelized_file<int> lf(path_prefix);

            AssertThat(lf.is_persistent(), Is().True());
            AssertThat(lf.is_temp(), Is().False());
          }

          // Cleanup after this test
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");
        });

        it("has expected size(s) after reopening a persisted empty file", [&tmp_path]() {
          const std::string path_prefix = tmp_path + "existing-prefix.adiar";

          // Clean up after prior tests
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          {
            levelized_file<int> lf;
            lf.make_persistent(path_prefix);
          }
          {
            levelized_file<int> lf(path_prefix);

            AssertThat(lf.size(), Is().EqualTo(0u));
            AssertThat(lf.size(0), Is().EqualTo(0u));
            AssertThat(lf.size(1), Is().EqualTo(0u));
            AssertThat(lf.levels(), Is().EqualTo(0u));
          }

          // Cleanup after this test
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");
        });

        // Customly construct a non-empty levelized file without using the
        // 'levelized_ofstream'.
        const std::string path_prefix = tmp_path + "existing-prefix.adiar";

        // Clean up after prior tests
        if (std::filesystem::exists(path_prefix + ".file_0"))
          std::filesystem::remove(path_prefix + ".file_0");
        if (std::filesystem::exists(path_prefix + ".file_1"))
          std::filesystem::remove(path_prefix + ".file_1");
        if (std::filesystem::exists(path_prefix + ".levels"))
          std::filesystem::remove(path_prefix + ".levels");

        {
          levelized_file<int> lf;
          levelized_ofstream<int> lfw(lf);

          lfw.push<0>(0);
          lfw.push<1>(42);
          lfw.push<0>(2);
          lfw.push<0>(8);
          lfw.push(level_info{ 0, 4 });
          lfw.close();

          lf.make_persistent(path_prefix);
        }

        it("can reopen a persisted non-empty file",
           [&path_prefix]() { levelized_file<int> lf(path_prefix); });

        it("has expected size(s) after reopening a persisted non-empty file", [&path_prefix]() {
          levelized_file<int> lf(path_prefix);
          AssertThat(lf.size(), Is().EqualTo(4u));
          AssertThat(lf.size(0), Is().EqualTo(3u));
          AssertThat(lf.size(1), Is().EqualTo(1u));
          AssertThat(lf.levels(), Is().EqualTo(1u));
        });

        it("has the non-empty file still marked persistent", [&path_prefix]() {
          levelized_file<int> lf(path_prefix);
          AssertThat(lf.is_persistent(), Is().True());
          AssertThat(lf.is_temp(), Is().False());
        });

        it("can read content with a stream", [&path_prefix]() {
          levelized_file<int> lf(path_prefix);
          levelized_ifstream<int> lfs(lf);

          AssertThat(lfs.pull<0>(), Is().EqualTo(0));
          AssertThat(lfs.pull<0>(), Is().EqualTo(2));
          AssertThat(lfs.pull<0>(), Is().EqualTo(8));
          AssertThat(lfs.pull<1>(), Is().EqualTo(42));
        });

        it("cannot reattach a writer to a persisted file", [&path_prefix]() {
          levelized_file<int> lf(path_prefix);
          levelized_ofstream<int> lfw;
          AssertThrows(runtime_error, lfw.open(lf));
        });

        it("is unchanged after marking it persistent once more", [&path_prefix]() {
          {
            levelized_file<int> lf(path_prefix);
            lf.make_persistent();

            AssertThat(lf.is_persistent(), Is().True());
            AssertThat(lf.is_temp(), Is().False());
          }

          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().True());
          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().True());
        });

        it("cannot be 'moved'", [&tmp_path, &path_prefix]() {
          levelized_file<int> lf(path_prefix);
          lf.make_persistent();

          AssertThat(lf.can_move(), Is().False());
          AssertThrows(runtime_error, lf.move(tmp_path + "alternative-prefix.adiar"));
        });

        // TODO: test data is persisted.

        // Clean up customly constructed persisted non-empty file after its tests
        if (std::filesystem::exists(path_prefix + ".file_0"))
          std::filesystem::remove(path_prefix + ".file_0");
        if (std::filesystem::exists(path_prefix + ".file_1"))
          std::filesystem::remove(path_prefix + ".file_1");
        if (std::filesystem::exists(path_prefix + ".levels"))
          std::filesystem::remove(path_prefix + ".levels");
      });

      describe("levelized_file.sort(const pred&, size_t idx)", [&tmp_path]() {
        it("can sort an empty subfile [0]", []() {
          levelized_file<int> lf;
          lf.touch();

          AssertThat(lf.empty(), Is().True());

          lf.sort<std::less<int>>(0);

          AssertThat(lf.empty(), Is().True());
        });

        it("can sort an empty subfile [1]", []() {
          levelized_file<int> lf;
          lf.touch();

          AssertThat(lf.empty(), Is().True());

          lf.sort<std::less<int>>(1);

          AssertThat(lf.empty(), Is().True());
        });

        it("cannot sort an index out of bounds", []() {
          levelized_file<int> lf;
          lf.touch();

          AssertThrows(out_of_range, lf.sort<std::less<int>>(2));
        });

        it("can sort a temporary non-empty file", []() {
          levelized_file<int> lf;

          levelized_ofstream<int> lfw(lf);
          lfw.push<0>(-1);
          lfw.push<1>(0);
          lfw.push<0>(2);
          lfw.push<0>(1);
          lfw.push<1>(-1);
          lfw.push<1>(2);
          lfw.push<0>(0);
          lfw.push<1>(1);
          lfw.push(level_info{ 0, 1 });
          lfw.push(level_info{ 1, 2 });
          lfw.push(level_info{ 2, 4 });
          lfw.push(level_info{ 3, 1 });
          lfw.close();

          { // Check is unsorted first
            levelized_ifstream<int> lfs(lf);
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(-1));
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(2));
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(1));
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(0));
            AssertThat(lfs.can_pull<0>(), Is().False());

            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(0));
            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(-1));
            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(2));
            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(1));
            AssertThat(lfs.can_pull<1>(), Is().False());
          }

          lf.sort<std::less<int>>(0);
          { // Check has (only) the first file sorted
            levelized_ifstream<int> lfs(lf);

            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(-1));
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(0));
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(1));
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(2));
            AssertThat(lfs.can_pull<0>(), Is().False());

            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(0));
            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(-1));
            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(2));
            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(1));
            AssertThat(lfs.can_pull<1>(), Is().False());
          }

          lf.sort<std::less<int>>(1);
          { // Check has both files sorted
            levelized_ifstream<int> lfs(lf);

            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(-1));
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(0));
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(1));
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(2));
            AssertThat(lfs.can_pull<0>(), Is().False());

            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(-1));
            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(0));
            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(1));
            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(2));
            AssertThat(lfs.can_pull<1>(), Is().False());
          }
        });

        it("cannot sort a persisted empty file", [&tmp_path]() {
          const std::string path_prefix = tmp_path + "sort-prefix.adiar";

          // Clean up after prior tests
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          { // Scope to destruct 'lf' early
            levelized_file<int> lf;
            lf.make_persistent(path_prefix);

            AssertThrows(runtime_error, lf.sort<std::less<int>>(0));
            AssertThrows(runtime_error, lf.sort<std::less<int>>(1));
          }
          AssertThat(std::filesystem::exists(path_prefix + ".file_0"), Is().True());
          std::filesystem::remove(path_prefix + ".file_0");

          AssertThat(std::filesystem::exists(path_prefix + ".file_1"), Is().True());
          std::filesystem::remove(path_prefix + ".file_1");

          AssertThat(std::filesystem::exists(path_prefix + ".levels"), Is().True());
          std::filesystem::remove(path_prefix + ".levels");
        });

        it("cannot sort a persisted non-empty file", [&tmp_path]() {
          const std::string path_prefix = tmp_path + "sort-prefix.adiar";

          // Clean up after prior tests
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          { // Construct a persisted non-empty levelized file by hand
            levelized_file<int> lf;
            levelized_ofstream lfw(lf);

            lfw.push<0>(-1);
            lfw.push<0>(8);
            lfw.push<1>(4);
            lfw.push<0>(3);
            lfw.push<1>(2);
            lfw.push(level_info{ 2, 1 });
            lfw.push(level_info{ 1, 1 });

            lfw.close();

            lf.make_persistent(path_prefix);
          }

          { // Try to sort it
            levelized_file<int> lf(path_prefix);
            AssertThrows(runtime_error, lf.sort<std::less<int>>(0));
            AssertThrows(runtime_error, lf.sort<std::less<int>>(1));
          }

          { // Check content is not sorted
            levelized_file<int> lf(path_prefix);

            levelized_ifstream<int> lfs(lf);
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(-1));
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(8));
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(3));
            AssertThat(lfs.can_pull<0>(), Is().False());

            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(4));
            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(2));
            AssertThat(lfs.can_pull<1>(), Is().False());

            level_info_ifstream<true> fs_lvls(lf);
            AssertThat(fs_lvls.can_pull(), Is().True());
            AssertThat(fs_lvls.pull(), Is().EqualTo(level_info{ 2, 1 }));
            AssertThat(fs_lvls.can_pull(), Is().True());
            AssertThat(fs_lvls.pull(), Is().EqualTo(level_info{ 1, 1 }));
            AssertThat(fs_lvls.can_pull(), Is().False());
          }

          // Clean up for this test
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");
        });
      });

      describe("levelized_file::copy(const levelized_file&)", [&tmp_path]() {
        it("can copy over non-existing file", []() {
          levelized_file<int> lf1;

          // Set stats content
          lf1.meta_value = 42;

          const std::array<std::string, 2u + 1u> paths1 = lf1.paths();

          levelized_file<int> lf2                       = levelized_file<int>::copy(lf1);
          const std::array<std::string, 2u + 1u> paths2 = lf2.paths();

          // Check stats content
          AssertThat(lf2.meta_value, Is().EqualTo(42));

          // Check paths are unique
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

        it("is unaffected by orginal file being 'touched'", []() {
          levelized_file<int> lf1;
          levelized_file<int> lf2 = levelized_file<int>::copy(lf1);

          lf1.touch();
          AssertThat(lf1.exists(), Is().True());
          AssertThat(lf2.exists(), Is().False());
        });

        it("can copy an existing file [empty]", []() {
          levelized_file<int> lf1;

          // Set stats content and state
          lf1.meta_value = 7;
          lf1.touch();

          // Check file state
          AssertThat(lf1.exists(), Is().True());
          AssertThat(lf1.empty(), Is().True());
          AssertThat(lf1.size(), Is().EqualTo(0u));

          levelized_file<int> lf2 = levelized_file<int>::copy(lf1);

          // Check stats content
          AssertThat(lf2.meta_value, Is().EqualTo(7));

          // Check file state
          AssertThat(lf2.exists(), Is().True());
          AssertThat(lf2.empty(), Is().True());
          AssertThat(lf2.size(), Is().EqualTo(0u));

          // Check paths are unique
          const std::array<std::string, 2u + 1u> paths1 = lf1.paths();
          const std::array<std::string, 2u + 1u> paths2 = lf2.paths();

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

        it("can copy an existing file [non-empty]", []() {
          levelized_file<int> lf1;
          // Set file and stats content
          {
            levelized_ofstream<int> lfw(lf1);

            lfw.push<0>(21);
            lfw.push<1>(42);
            lfw.push<0>(21);
            lfw.push(level_info{ 0, 1 });
            lfw.push(level_info{ 1, 2 });
          }

          lf1.meta_value = 4; // <- very random number

          // Check state
          AssertThat(lf1.exists(), Is().True());
          AssertThat(lf1.empty(), Is().False());
          AssertThat(lf1.size(), Is().EqualTo(3u));
          AssertThat(lf1.size(0), Is().EqualTo(2u));
          AssertThat(lf1.size(1), Is().EqualTo(1u));
          AssertThat(lf1.levels(), Is().EqualTo(2u));

          levelized_file<int> lf2 = levelized_file<int>::copy(lf1);

          // Check stats content
          AssertThat(lf2.meta_value, Is().EqualTo(4));

          // Check state
          AssertThat(lf2.exists(), Is().True());
          AssertThat(lf2.empty(), Is().False());
          AssertThat(lf2.size(), Is().EqualTo(3u));
          AssertThat(lf2.size(0), Is().EqualTo(2u));
          AssertThat(lf2.size(1), Is().EqualTo(1u));
          AssertThat(lf2.levels(), Is().EqualTo(2u));

          // Check file content
          {
            levelized_ifstream<int> lfs(lf2);

            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(21));
            AssertThat(lfs.can_pull<0>(), Is().True());
            AssertThat(lfs.pull<0>(), Is().EqualTo(21));
            AssertThat(lfs.can_pull<0>(), Is().False());

            AssertThat(lfs.can_pull<1>(), Is().True());
            AssertThat(lfs.pull<1>(), Is().EqualTo(42));
            AssertThat(lfs.can_pull<1>(), Is().False());

            level_info_ifstream<true> fs_lvls(lf2);
            AssertThat(fs_lvls.can_pull(), Is().True());
            AssertThat(fs_lvls.pull(), Is().EqualTo(level_info{ 0, 1 }));
            AssertThat(fs_lvls.can_pull(), Is().True());
            AssertThat(fs_lvls.pull(), Is().EqualTo(level_info{ 1, 2 }));
            AssertThat(fs_lvls.can_pull(), Is().False());
          }

          // Check paths are unique
          const std::array<std::string, 2u + 1u> paths1 = lf1.paths();
          const std::array<std::string, 2u + 1u> paths2 = lf2.paths();

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

        it("can copy an existing persistent file", [&tmp_path]() {
          const std::string path_prefix = tmp_path + "sort-prefix.adiar";

          // Clean up after prior tests
          if (std::filesystem::exists(path_prefix + ".file_0"))
            std::filesystem::remove(path_prefix + ".file_0");
          if (std::filesystem::exists(path_prefix + ".file_1"))
            std::filesystem::remove(path_prefix + ".file_1");
          if (std::filesystem::exists(path_prefix + ".levels"))
            std::filesystem::remove(path_prefix + ".levels");

          levelized_file<int> lf1;

          // Set stats content and state
          {
            levelized_ofstream<int> lfw(lf1);

            lfw.push<0>(21);
            lfw.push<1>(42);
            lfw.push<1>(21);
            lfw.push(level_info{ 0, 1 });
            lfw.push(level_info{ 1, 2 });
          }
          lf1.meta_value = 21;
          lf1.make_persistent(path_prefix);

          // Check state
          AssertThat(lf1.is_persistent(), Is().True());
          AssertThat(lf1.is_temp(), Is().False());

          AssertThat(lf1.exists(), Is().True());
          AssertThat(lf1.empty(), Is().False());

          AssertThat(lf1.size(), Is().EqualTo(3u));
          AssertThat(lf1.size(0), Is().EqualTo(1u));
          AssertThat(lf1.size(1), Is().EqualTo(2u));
          AssertThat(lf1.levels(), Is().EqualTo(2u));

          levelized_file<int> lf2 = levelized_file<int>::copy(lf1);

          // Check stats content
          AssertThat(lf2.meta_value, Is().EqualTo(21));

          // Check state
          AssertThat(lf2.is_persistent(), Is().False());
          AssertThat(lf2.is_temp(), Is().True());

          AssertThat(lf2.exists(), Is().True());
          AssertThat(lf2.empty(), Is().False());

          AssertThat(lf2.size(), Is().EqualTo(3u));
          AssertThat(lf2.size(0), Is().EqualTo(1u));
          AssertThat(lf2.size(1), Is().EqualTo(2u));
          AssertThat(lf2.levels(), Is().EqualTo(2u));

          // Check paths are unique
          const std::array<std::string, 2u + 1u> paths1 = lf1.paths();
          const std::array<std::string, 2u + 1u> paths2 = lf2.paths();

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
      });
    });
});
