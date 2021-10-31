#ifndef TSNE_RUNNER_H
#define TSNE_RUNNER_H

namespace TSNE {

    void customRun(double* X, int N, int D, double* Y, int no_dims, double perplexity, double theta, int rand_seed,
            bool skip_random_init, int max_iter, int stop_lying_iter, int mom_switch_iter, std::vector<std::vector<double>>& iterations);

};

#endif
