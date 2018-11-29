# geff

The efficiency curve is determined by a simultaneous fit to all
experimental data. In the case that no normalisation data are
given at all, then it is assumed to be equal to 1 for the first
source and the other sources are fitted to this. In case multiple
normalisations are given for a source, the global fit will minimise
using all these values plus the constraints from the efficiency
data of the other sources and their experimental normalisations.
If source data given under efficiency doesn't have experimental
values of normalisation constants, it's sufficient to simply type
a dummy filename, i.e it doesn't have to exist. However, the
ordering of the sources under -n must match those under -e.


Usage: 
'''
geff -e <eff1.dat> -n <norm1.dat> ... -e <effX.dat> - n<normX.dat>
'''

effX.dat is the file containing the energy and efficiency
values for source number X. The format is 4 columns:
'''
   Energy (keV) | Error (keV) | Efficiency (arb.) | Error (arb.)
'''

normX.dat is the file containing experimentally determined
normalisation constants from arbitrary units to absolute %
for source number X. The format is 2 columns:
'''
   Normalisation (%/arb.) | Error (%/arb.)
'''
'''
geff --help
'''