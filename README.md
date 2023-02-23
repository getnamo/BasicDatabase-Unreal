# BasicDatabase-Unreal
Simple wrapped DB API for json flatfile database &amp; potentially more backends.

Aims to solve the binary file database with source control development problem. Small changes in a big binary file requires full file commit in e.g. git lfs. Typically databases are stored separately from source control. This plugin suggests an alternative: you can include minimally changing flat files during development, tracking your database along with your source in history.

Ideally this plugin will later have a conversion system to convert storage to something more production ready such as sqlite3 and support something like json diffs. There are also possible binary diff systems that may be more appropriate when such systems get adopted by common source control systems.

*Not yet functional* see: https://github.com/getnamo/BasicDatabase-Unreal/issues/1 for progress.
