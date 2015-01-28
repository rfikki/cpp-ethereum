## Building

Setup emscripten sdk.
Compile boost, leveldb and cryptopp in directories parallel to cpp-ethereum.
Run:
'''
emcmake cmake  -DETH_STATIC=1 -DLANGUAGES=1 -DHEADLESS=1
emmake make soljs
'''
