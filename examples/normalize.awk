#!/usr/bin/awk -f

{
	for (i = 1; i <= NF; ++i) {
	    split ($i, fields, "=");
	    map[fields[1]] = fields[2];
	}

	user = map["user"];
	total = map["total"];

	printf ("system=%f\n", (total - user) / total);
}
