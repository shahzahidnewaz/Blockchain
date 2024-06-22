# Gnuplot script for plotting "Man-in-the-Middle (MitM) Attack" data before imposing blockchain

# Set output file format and name
set terminal pngcairo enhanced font "arial,10" size 800, 600
set output "mitm_bar_chart.png"

# Set title and labels
set title "Impact of Man-in-the-Middle (MitM) Attack"
set xlabel "Time (s)"
set ylabel "Intercepted Packets"
set style fill solid
set boxwidth 0.5
set key top left

# Set data file format
# set datafile separator "\t"

# Plot intercepted packets before imposing blockchain
plot "mitm_data.dat" using 1:2 with boxes lw 2 title "Intercepted Packets (Before Blockchain)"
