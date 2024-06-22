# Gnuplot script for plotting comparative data before and after imposing blockchain to mitigate MitM attack

# Set output file format and name
set terminal pngcairo enhanced font "arial,10" size 800, 600
set output "mitm_fabric_comparison.png"

# Set title and labels
set title "Impact of Man-in-the-Middle (MitM) Attack with and without Blockchain"
set xlabel "Time (s)"
set ylabel "Intercepted Packets"
set style data histogram
set style histogram cluster gap 1
set style fill solid border -1
set boxwidth 0.4 relative

# Set data file format
# set datafile separator "\t"

# Plot the comparative data
plot "comparitive_dat_mitm.dat" using 2:xticlabels(1) title "Before Blockchain" lc rgb "blue", \
     '' using 3 title "After Blockchain" lc rgb "green"

