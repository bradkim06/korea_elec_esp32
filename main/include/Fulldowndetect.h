class Fulldowndetect {
   public:
    double fDiffAcc(double cAcc[]);
    void buffer_save(double cAcc[]);
    void buffer_diff2sum(double diffacc[]);
    bool fulldown_detect_result();

   private:
    double fPRc(double acc[]);
    double fdiff2sum(double diffacc[]);
    bool fFDalg(double mprval);
};
