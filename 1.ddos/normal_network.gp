# Gnuplot script for plotting packet transmission, reception, and loss as a bar chart

# Set output file format and name
set terminal pngcairo enhanced font "arial,10" size 800, 600
set output "1.normal_network.png"

# Set title and labels
set title "Packet Transmission, Reception, and Loss"
set xlabel "Time (s)"
set ylabel "Count"
set style fill solid
set boxwidth 0.5
set key top left

# Set data file format
# set datafile separator "\t"

# Plot packet transmission, reception, and loss as bars
plot "1.normal_network.dat" using ($0):2:xtic(1) with boxes lw 2 title "Packets Transmitted", \
     "1.normal_network.dat" using ($0+0.25):3 with boxes lw 2 title "Packets Received", \
     "1.normal_network.dat" using ($0+0.5):4 with boxes lw 2 title "Packet Loss"

