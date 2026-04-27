# STI3 Documentation Build and Deploy

The docs build produces one deployable site root:

```text
docs/build/sti3docs/
  index.html
  sphinx/
  doxygen/
```

`index.html` redirects to the Sphinx docs. The Doxygen API reference remains in
`doxygen/html/`, and Sphinx links to it with a relative URL.

## Build

From the repository root:

```bash
conda run --no-capture-output -n sti3-build cmake -S docs -B docs/build
conda run --no-capture-output -n sti3-build cmake --build docs/build --target DocsSite --parallel 8
```

Or from `docs/`:

```bash
./buildDocs
```

Useful targets:

```bash
cmake --build docs/build --target Sphinx
cmake --build docs/build --target Doxygen
cmake --build docs/build --target DocsSite
cmake --build docs/build --target Zip
```

The `Zip` target creates `docs/build/stidocs-3.0.1.tar.gz` containing the
deployable `sti3docs/` directory.

## Deploy

Deploy the archive created by the `Zip` target. This works well when the final
web directory requires `sudo`: copy the archive to a temporary location on the
server, then unpack it into the release directory with elevated permissions.

Do not deploy only the `sphinx/` directory; the Sphinx API page links to the
sibling `doxygen/html/` directory.

One release-oriented layout is:

```text
/var/projects/docs/sti3docs/releases/<release>/
  index.html
  sphinx/
  doxygen/

/var/projects/docs/sti3docs/current -> releases/<release>
/var/projects/trac/htdocs/sti3docs -> /var/projects/docs/sti3docs/current
```

Example:

```bash
scp docs/build/stidocs-3.0.1.tar.gz server:/tmp/

ssh server 'sudo mkdir -p /var/projects/docs/sti3docs/releases/2026-04-26'
ssh server 'sudo tar -xzf /tmp/stidocs-3.0.1.tar.gz -C /var/projects/docs/sti3docs/releases/2026-04-26 --strip-components=1'
ssh server 'sudo ln -sfn /var/projects/docs/sti3docs/releases/2026-04-26 /var/projects/docs/sti3docs/current'
ssh server 'sudo ln -sfn /var/projects/docs/sti3docs/current /var/projects/trac/htdocs/sti3docs'
```

The archive contains a top-level `sti3docs/` directory. The
`--strip-components=1` option removes that wrapper so the release directory
itself becomes the site root.

After this, the Trac URL should point at the site root:

```text
/sti3docs/
```

The top-level `index.html` redirects to:

```text
/sti3docs/sphinx/index.html
```

The Doxygen API reference is available at:

```text
/sti3docs/doxygen/html/index.html
```

If the deployment user has direct write permission to the release directory,
the same layout can also be deployed with `rsync`:

```bash
rsync -a docs/build/sti3docs/ server:/var/projects/docs/sti3docs/releases/2026-04-26/
```
