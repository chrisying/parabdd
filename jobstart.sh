#!/bin/bash

for i in {1..8}; do
    echo "#!/bin/bash" > parabdd-n$i.job
    echo "export CILK_NWORKERS=$i" >> parabdd-n$i.job
    cat parabdd.job >> parabdd-n$i.job
    qsub parabdd-n$i.job
done
