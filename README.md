# pure-markov: Markov Chains in Pure Data

> ...will this work?

## Development

> I assume you're on Mac because I'm not bothering to make this cross-platform.

Build from source:

```
make
```

Then move executable to same directory level as patch we're using. Here, the default build will be at the same directory level as `markov-test.pd`.

## Known Issues

- Relative paths are at root `/` instead of patch directory
- Not tested with more complicated CSVs
- Not tested with invalid CSVs
- Not checked for memory leaks
