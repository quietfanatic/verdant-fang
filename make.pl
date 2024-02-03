#!/usr/bin/perl
use lib do {__FILE__ =~ /^(.*)[\/\\]/; ($1||'.').'/src/external/make-pl'};
use MakePl;
use MakePl::C;
use File::Copy;
no warnings 'qw';

##### COMMAND LINE CONFIGURATION

my $compiler = 'gcc';
my %compilers;
my @linker;
my @includes;
my @compile_opts;
my @link_opts;
my @O0_opts;
my @O3_opts;
my @resources;

if ($compiler eq 'gcc') {
    %compilers = (
        cpp => ['g++-12'],
        c => ['gcc-12'],
    );
    @linker = 'g++-12';
    push @link_opts, qw(-lSDL2 -lSDL2_image -lSDL2_mixer);
}
elsif ($compiler eq 'mingw') {
    my $mingw = '../../programs/mingw';
    my $bin = '..\\..\\programs\\mingw\\mingw64\\bin';
    my $arch = 'x86_64-w64-mingw32';
    %compilers = (
        cpp => ["$bin\\g++.exe"],
        c => ["$bin\\gcc.exe"],
    );
    @linker = ("$bin\\g++.exe",
        qw(-lmingw32 -static-libgcc -static-libstdc++),
    );
     # These need to be at the end of the command line because the linker is
     # too dumb to look backwards.
    push @link_opts, (
        "-L$mingw/SDL2_image-2.8.2/$arch/lib",
        "-L$mingw/SDL2_mixer-2.8.0/$arch/lib",
        "-L$mingw/SDL2-2.28.5/$arch/lib",
        qw(-lSDL2_image -lSDL2_mixer -lSDL2main -lSDL2),
    );
    push @compile_opts, (
        "-I$mingw/SDL2-2.28.5/$arch/include",
         # The SDL sublibraries expect their headers to be in the same place as
         # the SDL headers, but I prefer to keep them separate, so here's an
         # ugly workaround.
        "-I$mingw/SDL2-2.28.5/$arch/include/SDL2",
        "-I$mingw/SDL2_image-2.8.2/$arch/include",
        "-I$mingw/SDL2_mixer-2.8.0/$arch/include",
    );
     # These are relative to ./src.  TODO: make this more intuitive
    push @resources, (
        ["../$mingw/SDL2-2.28.5/$arch/bin/SDL2.dll" => 'SDL2.dll'],
        ["../$mingw/SDL2_image-2.8.2/$arch/bin/SDL2_image.dll" => 'SDL2_image.dll'],
        ["../$mingw/SDL2_mixer-2.8.0/$arch/bin/SDL2_mixer.dll" => 'SDL2_mixer.dll'],
    );
}
else {
    die "Unsupported compiler option";
}

push @{$compilers{cpp}}, qw(
    -std=c++20 -fno-threadsafe-statics
    -ftemplate-backtrace-limit=0 -fconcepts-diagnostics-depth=4
);
push @compile_opts, qw(
    -msse2 -mfpmath=sse
    -fstrict-aliasing -fstack-protector
    -Wall -Wextra -Wno-unused-value
    -fmax-errors=10 -fdiagnostics-color -fno-diagnostics-show-caret
);
push @link_opts, qw();

 # Dead code elimination actually makes compilation slightly faster.
push @O0_opts, qw(-fdce);

 # MFW I discovered parallel LTO
push @O3_opts, qw(-O3 -flto=7);

$ENV{ASAN_OPTIONS} = 'new_delete_type_mismatch=0';
my %configs = (
    deb => {
        opts => [qw(-ggdb), @O0_opts],
    },
    opt => {
        opts => [qw(-DNDEBUG -ggdb), @O3_opts],
    },
    dog => {
        opts => [qw(-DTAP_DISABLE_TESTS -ggdb), @O3_opts],
    },
    rel => {
        opts => [qw(-DNDEBUG -DTAP_DISABLE_TESTS -s), @O3_opts],
    },
    opt32 => {
        opts => [qw(-m32 -fno-pie -DNDEBUG -ggdb), @O3_opts],
    },
    san => {
        opts => [qw(-ggdb), @O0_opts, '-fsanitize=address,undefined', '-fno-sanitize=enum'],
    },
    pro => {
        opts => [qw(-Og -DNDEBUG -flto -pg)],
    },
    ana => {
        opts => [qw(-ggdb -fanalyzer)],
        fork => 0,  # Absolutely thrashes RAM
    },
);

##### SOURCES

my $program = 'verdant-fang';

my @sources = (qw(
    vf/world/block.cpp
    vf/game/camera.cpp
    vf/game/commands.cpp
    vf/game/frame.cpp
    vf/game/game.cpp
    vf/game/main.cpp
    vf/world/player.cpp
    vf/game/room.cpp
    vf/game/settings.cpp
    vf/world/scenery.cpp
    vf/game/sound.cpp
    dirt/ayu/common.cpp
    dirt/ayu/data/parse.cpp
    dirt/ayu/data/print.cpp
    dirt/ayu/data/tree.cpp
    dirt/ayu/reflection/accessors.cpp
    dirt/ayu/reflection/describe-builtin.cpp
    dirt/ayu/reflection/describe-standard.cpp
    dirt/ayu/reflection/description.cpp
    dirt/ayu/reflection/dynamic.cpp
    dirt/ayu/reflection/pointer.cpp
    dirt/ayu/reflection/reference.cpp
    dirt/ayu/reflection/type.cpp
    dirt/ayu/resources/document.cpp
    dirt/ayu/resources/global.cpp
    dirt/ayu/resources/resource.cpp
    dirt/ayu/resources/scheme.cpp
    dirt/ayu/traversal/compound.cpp
    dirt/ayu/traversal/from-tree.cpp
    dirt/ayu/traversal/location.cpp
    dirt/ayu/traversal/scan.cpp
    dirt/ayu/traversal/test.cpp
    dirt/ayu/traversal/to-tree.cpp
    dirt/control/command.cpp
    dirt/control/command-builtins.cpp
    dirt/control/input.cpp
    dirt/geo/floating.t.cpp
    dirt/geo/mat.t.cpp
    dirt/geo/vec.t.cpp
    dirt/glow/colors.cpp
    dirt/glow/common.cpp
    dirt/glow/file-image.cpp
    dirt/glow/gl.cpp
    dirt/glow/image-texture.cpp
    dirt/glow/image-transform.cpp
    dirt/glow/image.cpp
    dirt/glow/program.cpp
    dirt/glow/resource-texture.cpp
    dirt/glow/test-environment.cpp
    dirt/glow/texture-program.cpp
    dirt/glow/texture.cpp
    dirt/iri/iri.cpp
    dirt/iri/path.cpp
    dirt/uni/arrays.t.cpp
    dirt/uni/assertions.cpp
    dirt/uni/errors.cpp
    dirt/uni/io.cpp
    dirt/uni/text.cpp
    dirt/uni/utf.cpp
    dirt/whereami/whereami.c
    dirt/wind/active_loop.cpp
    dirt/wind/window.cpp
),
    [qw(dirt/tap/tap.cpp -DTAP_SELF_TEST)],
);

push @resources, (qw(
    vf/game/*.ayu
    vf/world/*.ayu
    vf/world/*.png
    vf/world/*.mp3
),
    ["../LICENSE" => "LICENSE"],
    ["../README.md" => "README.md"],
);
my @test_resources = (qw(
    dirt/ayu/test/*.ayu
    dirt/ayu/test/*.json
    dirt/glow/test/*
    dirt/glow/texture-program.ayu
));

##### MISC

sub ensure_path {
    if ($_[0] =~ /^(.*)\//) {
        -d $1 or do {
            require File::Path;
            File::Path::make_path($1);
        }
    }
}

##### RULES

for my $cfg (keys %configs) {
     # Compile sources
    my @objects;
    for (@sources) {
        my $src = $_; # Don't alias
        my @opts = (@compile_opts, @{$configs{$cfg}{opts}});
        if (ref $_ eq 'ARRAY') {
            my @src_opts = (@$src);
            $src = shift @src_opts;
            push @opts, @src_opts;
        }

        $src =~ /(.*)\.([^\/.]+)$/ or die "No file extension in $src";
        my ($mod, $ext) = ($1, $2);
        my $compiler = $compilers{$ext} // die "Unrecognized source file extension in $src";

        rule "tmp/$cfg/$mod.o", "src/$src", sub {
            ensure_path($_[0][0]);
            run @$compiler, '-c', @{$_[1]},
                @opts,
                '-o', $_[0][0];
        }, {fork => $configs{$cfg}{fork} // 1};
        rule "tmp/$cfg/$mod.s", "src/$mod.cpp", sub {
            ensure_path($_[0][0]);
            run @$compiler, '-S', '-masm=intel', @{$_[1]},
                grep($_ ne '-ggdb' && $_ !~ /^-flto/, @opts),
                '-o', $_[0][0];
        }, {fork => $configs{$cfg}{fork} // 1};

        push @objects, "tmp/$cfg/$mod.o";
    }

     # Link program
    my $out_program = "out/$cfg/$program";
    rule $out_program, [@objects], sub {
        ensure_path $out_program;
        run @linker, @objects,
            @link_opts, @{$configs{$cfg}{opts} // []},
            '-o', $out_program;
        if ($cfg eq 'rel') {
            print "Final program size: ", -s $out_program, "\n";
        }
    }, {fork => 1};

     # Copy resources
    my $testing_disabled = grep $_ eq '-DTAP_DISABLE_TESTS', @{$configs{$cfg}{opts}};
    my @res = @resources;
    push @res, @test_resources unless $testing_disabled;
    my @out_resources;
    for (@res) {
        my $name = ref $_ eq 'ARRAY' ? $_->[0] : $_;
        my @files = glob "src/$name";
        @files or die "No resources matched $name\n";
        for my $from (@files) {
            my $to;
            if (ref $_ eq 'ARRAY') {
                $to = "out/$cfg/$_->[1]";
            }
            else {
                $to = $from;
                $to =~ s[^src/][out/$cfg/res/];
            }
            push @out_resources, $to;
            rule $to, $from, sub {
                ensure_path($to);
                copy($from, $to) or die "Copy failed: $!";
            }, {fork => 1};
        }
    }

     # Misc phonies
    phony "out/$cfg/build", [$out_program, @out_resources];
    phony "out/$cfg/test", "out/$cfg/build", sub {
        my $prog = $out_program;
        if ($compiler eq 'mingw') {
            $prog =~ s/\//\\/g;
            $prog = "$prog.exe";
        }
        run "$prog --test | perl -pe \"s/\\r//\" | prove -e \"$prog --test\" -";
    };
}

phony 'debug', 'out/deb/build';
phony 'release', 'out/rel/build';
phony 'test', 'out/deb/test';
defaults 'test';

phony 'clean', [], sub {
    require File::Path;
    File::Path::remove_tree('tmp');
    File::Path::remove_tree('out');
};

make;
