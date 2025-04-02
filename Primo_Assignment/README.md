Assignment svolto dal gruppo 25:

Richiesta:
Implementare tre passi LLVM che realizzano le seguenti ottimizzazioni:
• 1. Algebraic Identity
      𝑥 + 0 = 0 + 𝑥 ⇒𝑥
      𝑥 × 1 = 1 × 𝑥 ⇒𝑥
• 2. Strength Reduction (più avanzato)
      15 × 𝑥 = 𝑥 × 15 ⇒ (𝑥 ≪ 4) – x
      y = x / 8 ⇒ y = x >> 3
• 3. Multi-Instruction Optimization
      𝑎 = 𝑏 + 1, 𝑐 = 𝑎 − 1 ⇒𝑎 = 𝑏 + 1, 𝑐 = 𝑏


Il file MyPass.cpp contiene tutte e 3 le richieste, mentre il CMakeLists contiene le istruzioni per compilare il tutto. FileTest è la cartella con i file di test. Se si vuole creare un nuova build, basta creare una nuova cartella (Che si trovi insieme a CMakeLists.txt e MyPass.cpp) ed eseguire il cmake e il make successivamente.
