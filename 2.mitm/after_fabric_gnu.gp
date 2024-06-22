# Gnuplot script for plotting "Man-in-the-Middle (MitM) Attack" data after imposing blockchain

# Set output file format and name
set terminal pngcairo enhanced font "arial,10" size 800, 600
set output "after_fabric_bar_chart.png"

# Set title and labels
set title "Impact of Man-in-the-Middle (MitM) Attack (After Blockchain)"
set xlabel "Time (s)"
set ylabel "Intercepted Packets"
set style fill solid
set boxwidth 0.5
set key top left

# Set data file format
# set datafile separator "\t"

# Plot intercepted packets after imposing blockchain
plot "after_fabric.dat" using 1:2 with boxes lw 2 title "Intercepted Packets (After Blockchain)"
