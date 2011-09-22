#!/usr/bin/awk -f

{
	for (i = 1; i <= NF; ++i) {
	    split ($i, fields, "=");
	    s1[fields[1]] += fields[2];
	    s2[fields[1]] += (fields[2] * fields[2]);
	}
}

END {
    for (i in s1) {
    	avg[i] = s1[i] / NR;
	dev[i] = sqrt (NR * s2[i] - s1[i] * s1[i]) / NR / sqrt (NR) * 1.96; # 95% Confidence interval.
    }

    for (i in avg) {
    	printf ("%s %0.12f %0.12f\n", i, avg[i], dev[i]);
    }
}