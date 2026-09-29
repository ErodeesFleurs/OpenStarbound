#!/bin/sh -eu

triplet_dir=$1
cp -L "$triplet_dir/lib/libjemalloc.so.2" dist/libjemalloc.so.2
cp "$triplet_dir/share/jemalloc/copyright" dist/jemalloc-LICENSE

# Utility executables lack the client/server targets' explicit $ORIGIN rpath.
for tool in asset_packer asset_unpacker btree_repacker dump_versioned_json make_versioned_json; do
  patchelf --add-rpath '$ORIGIN' "dist/$tool"
done

./dist/asset_packer -version
./dist/starbound_server -version
