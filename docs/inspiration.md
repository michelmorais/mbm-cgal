# CGAL: mapa de possibilidades

Este documento resume, em português, os 126 pacotes distribuídos em 18 áreas no [índice oficial de pacotes da CGAL](https://doc.cgal.org/latest/Manual/packages.html), consultado em 2026-09-25. O índice consultado corresponde à CGAL 6.2.1. Os nomes oficiais em inglês foram mantidos para facilitar a busca nos manuais.

O foco da análise é o uso da CGAL como conjunto de ferramentas **offline e separado da mini-mbm**. Isto não é uma proposta para vincular CGAL à execução da engine. Uma indicação de aderência mede o quanto um pacote pode apoiar as ferramentas deste repositório ou os fluxos de recursos da engine; não é uma avaliação de qualidade ou desempenho do algoritmo.

## Como ler a aderência

- **Alta:** há um caso de uso plausível em processamento de malhas, geometria 2D, validação ou geração de assets.
- **Média:** pode ser útil em uma ferramenta específica, dependendo de necessidades ainda não confirmadas.
- **Baixa:** área especializada, sem benefício evidente para os fluxos atuais da engine.
- **Infraestrutura:** abstração ou estrutura de suporte; útil principalmente para implementar outros pacotes.

O README deste projeto exige CGAL >= 6.0. O índice `latest` inclui pacotes introduzidos depois disso; eles podem exigir atualizar e testar a dependência antes de serem usados. Os casos identificáveis no próprio índice são assinalados abaixo.

## Aritmética e álgebra

Conceitos de tipos numéricos e ferramentas algébricas usados por algoritmos geométricos. Em geral, são dependências internas, não recursos de usuário.

- **Algebraic Foundations** (fundamentos algébricos) — define conceitos e interfaces para os tipos algébricos usados pela CGAL. **Aderência: Infraestrutura.**
- **Number Types** (tipos numéricos) — oferece tipos numéricos e adaptadores, inclusive para aritmética exata. **Aderência: Infraestrutura.**
- **Modular Arithmetic** (aritmética modular) — cálculos em corpos finitos, úteis em filtros e algoritmos algébricos. **Aderência: Baixa.**
- **Polynomial** (polinômios) — representação e operações com polinômios em uma ou várias variáveis. **Aderência: Baixa.**
- **Algebraic Kernel** (núcleo algébrico) — compara e aproxima raízes reais de polinômios; a documentação indica suporte concreto para polinômios univariados. **Aderência: Baixa.**

## Algoritmos combinatórios

Técnicas gerais de busca e otimização discreta. Podem apoiar soluções especializadas, mas não aparecem como necessidades imediatas do pipeline de malhas.

- **Monotone and Sorted Matrix Search** (busca em matrizes ordenadas) — busca eficiente em matrizes com propriedades de ordenação; é uma técnica usada em problemas geométricos específicos. **Aderência: Baixa.**
- **Linear and Quadratic Programming Solver** (programação linear e quadrática) — resolve problemas de otimização com restrições lineares e funções lineares ou quadráticas convexas, inclusive com resultados certificados. **Aderência: Média** para futuras ferramentas de otimização geométrica; provavelmente indireta.

## Núcleos geométricos

Tipos fundamentais de pontos, retas, segmentos, círculos e predicados. Estes pacotes são a base para implementar geometria robusta.

- **2D and 3D Linear Geometry Kernel** (núcleo de geometria linear 2D/3D) — fornece primitivas geométricas e predicados com diferentes compromissos entre exatidão e desempenho. **Aderência: Alta** como base dos workers existentes.
- **dD Geometry Kernel** (núcleo geométrico em dimensão d) — generaliza primitivas geométricas para dimensão arbitrária. **Aderência: Baixa.**
- **2D Circular Geometry Kernel** (núcleo de geometria circular 2D) — trata círculos, arcos circulares e segmentos no plano com construções robustas. **Aderência: Média** para ferramentas de contorno e desenho vetorial.
- **3D Spherical Geometry Kernel** (núcleo de geometria esférica 3D) — operações exatas ou robustas sobre esferas, círculos e arcos no espaço. **Aderência: Baixa.**

## Algoritmos de envoltória convexa

A envoltória convexa é a menor forma convexa que contém um conjunto de pontos. É útil, por exemplo, como primeiro tipo de proxy de colisão.

- **2D Convex Hulls and Extreme Points** (envoltórias convexas e extremos 2D) — calcula a envoltória e pontos extremos de conjuntos planares. **Aderência: Média** para contornos e colisores 2D.
- **3D Convex Hulls** (envoltórias convexas 3D) — calcula e testa envoltórias convexas espaciais. **Aderência: Alta** para gerar proxies convexos simples para Bullet.
- **dD Convex Hulls and Delaunay Triangulations** (envoltórias e triangulações Delaunay em dimensão d) — generaliza esses algoritmos para várias dimensões. **Aderência: Baixa.**

## Polígonos

Operações 2D diretamente relacionadas a contornos, furos, formas de colisão, recortes e ferramentas vetoriais.

- **2D Polygons** (polígonos 2D) — testa orientação, convexidade, simplicidade, área e localização de pontos. **Aderência: Alta** para validar contornos e entradas do editor de image mesh.
- **2D Polygon Repair** (reparo de polígonos 2D) — resolve contornos inválidos ou auto-intersectantes selecionando regiões segundo regras como par-ímpar. Introduzido na CGAL 6.0. **Aderência: Alta** para diagnóstico e limpeza explícita de contornos.
- **2D Regularized Boolean Set-Operations** (operações booleanas regulares 2D) — união, interseção e diferença de regiões delimitadas por curvas. **Aderência: Alta** para edição de formas, recortes e contornos de colisão.
- **2D Boolean Operations on Nef Polygons** (operações booleanas com polígonos Nef) — representação geral de conjuntos planares fechados sob operações booleanas. **Aderência: Baixa** para uso comum; pode ser útil quando a topologia exige maior generalidade.
- **2D Boolean Operations on Nef Polygons Embedded on the Sphere** (polígonos Nef na esfera) — operações análogas sobre regiões da esfera. **Aderência: Baixa.**
- **2D Polygon Partitioning** (particionamento de polígonos) — divide polígonos em peças convexas ou monótonas. **Aderência: Alta** para converter silhuetas em colisores convexos 2D ou decompor formas complexas.
- **2D Straight Skeleton and Polygon Offsetting** (esqueleto reto e deslocamento de polígonos) — calcula a estrutura interna gerada pelo avanço das arestas e cria offsets para dentro ou para fora. **Aderência: Média** para contornos de colisão, margens e ferramentas de forma.
- **2D Minkowski Sums** (somas de Minkowski 2D) — combina formas; a soma com um disco equivale a expandir um contorno. **Aderência: Alta** para expandir caixas de colisão, criar margens e testar colisões por forma.
- **2D Polyline Simplification** (simplificação de polilinhas 2D) — remove pontos mantendo a topologia da linha e respeitando critérios de custo/parada. **Aderência: Alta** para reduzir contornos de sprites sem alterar suas conexões.
- **dD Fréchet Distance** (distância de Fréchet em dimensão d) — aproxima uma medida de semelhança entre curvas que considera sua ordem ao longo do percurso. Introduzido na CGAL 6.1. **Aderência: Baixa**, exceto para comparar trajetórias ou contornos.
- **2D Visibility Computation** (visibilidade 2D) — calcula a região visível a partir de um ponto dentro de um domínio poligonal. **Aderência: Média** para protótipos de visão/oclusão em jogos 2D, não necessariamente para runtime.
- **2D Movable Separability of Sets** (separabilidade móvel de conjuntos) — estuda se conjuntos planares podem ser movimentados sem colidir, sob modelos específicos de movimento. **Aderência: Baixa.**

## Complexos celulares e poliedros

Estruturas topológicas que representam superfícies e volumes por vértices, arestas, faces e células. A maior parte é infraestrutura para algoritmos 3D avançados.

- **3D Polyhedral Surface** (superfície poliédrica 3D) — estrutura de meia-arestas para superfícies orientáveis que, localmente, se comportam como um disco, com ou sem bordas. **Aderência: Infraestrutura.**
- **Halfedge Data Structures** (estruturas de meia-arestas) — representa relações de incidência e vizinhança entre elementos de uma superfície. **Aderência: Infraestrutura.**
- **Surface Mesh** (malha de superfície) — estrutura indexada e flexível para superfícies poligonais; já é usada pelo worker deste projeto. **Aderência: Alta** como base das ferramentas offline.
- **Combinatorial Maps** (mapas combinatórios) — representa subdivisões orientáveis de várias dimensões, associando atributos às células. **Aderência: Baixa** fora de modelagem topológica especializada.
- **Generalized Maps** (mapas generalizados) — estrutura para subdivisões orientáveis ou não orientáveis e suas relações de adjacência. **Aderência: Baixa.**
- **Linear Cell Complex** (complexos celulares lineares) — associa geometria linear a mapas celulares para representar subdivisões em várias dimensões. **Aderência: Baixa.**
- **3D Boolean Operations on Nef Polyhedra** (operações booleanas com poliedros Nef) — operações gerais sobre sólidos, inclusive configurações não-manifold e elementos de dimensões distintas. **Aderência: Média** para ferramentas CAD avançadas; provavelmente complexidade excessiva para o fluxo inicial de assets.
- **Convex Decomposition of Polyhedra** (decomposição convexa de poliedros) — cobre um sólido com várias peças convexas, exatamente ou de forma aproximada. **Aderência: Alta** para o futuro gerador de proxies de colisão Bullet.
- **3D Minkowski Sum of Polyhedra** (soma de Minkowski 3D) — combina sólidos e permite modelar volumes varridos ou espaços de configuração. **Aderência: Baixa/Média** para geração offline de colisores e ferramentas geométricas.

## Arranjos

Um arranjo subdivide o plano ou o espaço em regiões induzidas por curvas/superfícies que se cruzam. É uma base importante para booleanas, interseções e reparo 2D.

- **2D Arrangements** (arranjos 2D) — constrói e mantém subdivisões planares, com consultas de localização e sobreposição. **Aderência: Média** como fundamento das ferramentas 2D.
- **2D Intersection of Curves** (interseção de curvas 2D) — encontra cruzamentos e subcurvas não sobrepostas por varredura do plano. **Aderência: Alta** para validação e reparo de contornos.
- **2D Snap Rounding** (arredondamento para grade 2D) — converte geometria de precisão arbitrária para uma grade finita, controlando relações entre arestas e vértices. **Aderência: Média** para importar contornos ruidosos, desde que a perda de precisão seja explícita.
- **2D Envelopes** (envelopes 2D) — calcula qual curva fica acima ou abaixo das demais em cada intervalo. **Aderência: Baixa.**
- **3D Envelopes** (envelopes 3D) — determina qual superfície define o envelope em cada região de uma subdivisão planar. **Aderência: Baixa.**

## Triangulações e triangulações de Delaunay

Estruturas para subdividir pontos em triângulos ou tetraedros, mantendo relações de vizinhança. Delaunay, versões com restrições e formas alpha têm usos mais próximos dos assets.

- **2D Triangulations** (triangulações 2D) — triangulações comuns, Delaunay, regulares e com segmentos obrigatórios; também servem a consultas de vizinho e diagramas duais. **Aderência: Alta** para geração de malhas 2D e contornos com restrições.
- **2D Triangulation Data Structure** (estrutura de dados de triangulação 2D) — armazena a topologia de triangulações e suas operações combinatórias. **Aderência: Infraestrutura.**
- **2D Triangulations on the Sphere** (triangulações 2D na esfera) — triangulações de Delaunay na superfície esférica. **Aderência: Baixa.**
- **2D Periodic Triangulations** (triangulações periódicas 2D) — triangula o toro plano, conectando as bordas opostas de um domínio periódico. **Aderência: Baixa**, exceto para mapas/tile sets periódicos específicos.
- **2D Hyperbolic Delaunay Triangulations** (Delaunay hiperbólica 2D) — triangulações no modelo de disco de Poincaré. **Aderência: Baixa; pesquisa/especialidade.**
- **2D Triangulations on Hyperbolic Surfaces** (triangulações em superfícies hiperbólicas) — gera e manipula triangulações de superfícies hiperbólicas fechadas. Introduzido na CGAL 6.1. **Aderência: Baixa.**
- **2D Periodic Hyperbolic Triangulations** (triangulações hiperbólicas periódicas) — triangulações periódicas em uma superfície hiperbólica específica. **Aderência: Baixa.**
- **3D Triangulations** (triangulações 3D) — triangulações de Delaunay e regulares em 3D, com inserção/remoção incremental e consultas espaciais. **Aderência: Média** para meshing e geometria sólida.
- **3D Triangulation Data Structure** (estrutura de dados de triangulação 3D) — armazena a topologia de triangulações volumétricas. **Aderência: Infraestrutura.**
- **3D Constrained Triangulations** (triangulações 3D com restrições) — constrói uma triangulação que incorpora as faces de uma superfície complexa; pode inserir pontos auxiliares e nem todo domínio é tetraedrizável. Introduzido na CGAL 6.1. **Aderência: Média** para malhas volumétricas, não para simplificação de superfície.
- **3D Periodic Triangulations** (triangulações periódicas 3D) — triangula um domínio 3D periódico, como um toro plano. **Aderência: Baixa.**
- **dD Triangulations** (triangulações em dimensão d) — estrutura e algoritmos de triangulação para dimensão definida em compilação ou execução. **Aderência: Baixa.**
- **2D Alpha Shapes** (formas alpha 2D) — extrai contornos de um conjunto de pontos conforme uma escala, permitindo revelar concavidades e componentes. **Aderência: Média** para contornos de nuvens de pontos ou imagens amostradas.
- **3D Alpha Shapes** (formas alpha 3D) — equivalente em 3D, com uma família de formas que varia conforme a escala. **Aderência: Média** para reconstrução e processamento de nuvens de pontos.

## Diagramas de Voronoi

Diagramas de Voronoi dividem o espaço conforme o elemento mais próximo. São úteis para espaçamento, regiões de influência e algumas formas de geração procedural.

- **2D Segment Delaunay Graphs** (grafos Delaunay de segmentos 2D) — dual de Voronoi para conjuntos de pontos e segmentos. **Aderência: Média** para análise de distância a contornos.
- **L Infinity Segment Delaunay Graphs** (Delaunay de segmentos na métrica L-infinito) — versão baseada na distância máxima entre coordenadas, em vez da distância euclidiana. **Aderência: Baixa.**
- **2D Apollonius Graphs (Delaunay Graphs of Disks)** (grafos de Apolônio para discos) — Voronoi aditivamente ponderado, em que cada local é um disco com raio próprio. **Aderência: Média** para distância a objetos com espessura/raio.
- **2D Voronoi Diagram Adaptor** (adaptador de diagramas de Voronoi 2D) — deriva o diagrama de Voronoi de uma triangulação Delaunay e permite consultas de região. **Aderência: Média** para ferramentas de distribuição procedural, não prioridade do processador atual.

## Geração de malhas

Gera malhas de superfície ou de volume a partir de contornos, funções, imagens ou domínios. Este grupo contém possibilidades diferentes do simplificador planar atual.

- **2D Conforming Triangulations and Meshes** (triangulações conformes e malhas 2D) — refina uma triangulação Delaunay com contornos obrigatórios até atender critérios de tamanho e forma dos triângulos; inclui o método de Lloyd, que melhora a distribuição dos vértices. **Aderência: Alta** para comparar com a triangulação da malha de imagem.
- **2D Alpha Wrapping** (envoltória alpha 2D) — produz polígonos válidos que envolvem pontos, segmentos ou polígonos, com controle de detalhe por parâmetros. Introduzido na CGAL 6.2. **Aderência: Média** para gerar contornos robustos a partir de amostras.
- **3D Surface Mesh Generation** (geração de malha de superfície 3D) — aproxima superfícies suaves definidas implicitamente ou por imagens volumétricas, controlando tamanho, forma e erro. **Aderência: Média** para gerar objetos ou superfícies de colisão a partir de campos.
- **3D Skin Surface Meshing** (malhas de superfícies skin) — gera superfícies suaves definidas por conjuntos de esferas e um parâmetro que controla como elas se unem; uma aplicação de origem é a modelagem molecular. **Aderência: Baixa.**
- **3D Simplicial Mesh Data Structure** (estrutura para malhas simpliciais 3D) — guarda tetraedros e subcomplexos para geração/remalhamento de volumes. Introduzido na CGAL 5.6. **Aderência: Baixa/Média** se surgirem necessidades de malha volumétrica.
- **3D Mesh Generation** (geração de malhas volumétricas 3D) — discretiza domínios 3D em tetraedros, preservando fronteiras, subdomínios e features geométricas. **Aderência: Baixa** para uma engine de jogos; pode servir a simulação ou análise offline.
- **3D Isosurfacing** (extração de isossuperfícies 3D) — gera superfícies de campos escalares em grade, por Marching Cubes, variantes topologicamente corretas ou Dual Contouring. Introduzido na CGAL 6.1. **Aderência: Média** para converter dados volumétricos em assets.
- **Tetrahedral Remeshing** (remalhamento tetraédrico) — melhora a qualidade e uniformidade de tetraedros por divisões, colapsos, flips e deslocamentos, mantendo features geométricas. **Aderência: Baixa/Média** para aplicações de simulação volumétrica.
- **3D Periodic Mesh Generation** (geração de malha periódica 3D) — gera malhas tetraédricas em domínios periódicos 3D. **Aderência: Baixa.**
- **3D Alpha Wrapping** (envoltória alpha 3D) — envolve uma malha, conjunto de triângulos ou nuvem de pontos com uma superfície fechada, sem auto-interseções e localmente semelhante a uma esfera; troca fidelidade por simplicidade via parâmetros. **Aderência: Alta** para gerar proxies ou fechar superfícies; verificar a cobertura e o afastamento antes de usar o resultado como colisor.

## Reconstrução de formas

Reconstrói curvas ou superfícies a partir de nuvens de pontos não ordenadas. É relevante se a mini-mbm passar a oferecer importação de scanner, fotogrametria ou dados 3D medidos.

- **Poisson Surface Reconstruction** (reconstrução de superfície de Poisson) — reconstrói uma superfície implícita a partir de pontos com normais orientadas. **Aderência: Média** para uma futura ferramenta de importação de scan.
- **Scale-Space Surface Reconstruction** (reconstrução em espaço de escala) — reconstrói uma superfície interpolante a partir de pontos, usando alpha shapes ou advancing front e um parâmetro de escala. **Aderência: Média** para nuvens de pontos.
- **Advancing Front Surface Reconstruction** (reconstrução por frente progressiva) — cresce uma superfície triangular a partir de uma nuvem não ordenada, escolhendo faces plausíveis sem criar certas singularidades topológicas. **Aderência: Média** para protótipo de reconstrução.
- **Polygonal Surface Reconstruction** (reconstrução de superfícies poligonais) — reconstrói modelos aproximadamente planares e leves a partir de pontos e segmentos planares, buscando recuperar arestas vivas. **Aderência: Média/Alta** para assets arquitetônicos low-poly, caso haja entrada de scans.
- **Kinetic Space Partition** (particionamento cinético do espaço) — divide a caixa envolvente de formas planares em volumes convexos e controla a complexidade. **Aderência: Baixa.**
- **Kinetic Surface Reconstruction** (reconstrução cinética de superfície) — pipeline de reconstrução aproximadamente planar a partir de nuvens de pontos, combinando detecção/regularização de formas e particionamento. **Aderência: Média** para importação de scans arquitetônicos.
- **Optimal Transportation Curve Reconstruction** (reconstrução de curvas por transporte ótimo) — reconstrói e simplifica curvas 2D a partir de pontos ruidosos ou com outliers. **Aderência: Média** para contornos vetoriais extraídos de imagem.

## Processamento de malhas poligonais

É a área mais próxima do worker existente: validação, reparo, booleanas, remalhamento, simplificação, parametrização e análise de superfícies.

- **Polygon Mesh Processing** (processamento de malhas poligonais) — conjunto amplo de operações, incluindo medidas, distâncias, interseções e consultas. **Aderência: Alta** como caixa de ferramentas para workers offline.
- **Boolean Operations on Meshes** (operações booleanas em malhas) — união, interseção, diferença, clipping, divisão e corte de superfícies trianguladas. **Aderência: Alta** para uma futura ferramenta de modelagem/recorte offline.
- **Meshing and Remeshing of Polygon Meshes** (geração e remalhamento de malhas poligonais) — triangula faces, refina, simplifica, otimiza e suaviza malhas. Inclui o remalhamento isotrópico: redistribui/regulariza triângulos em torno da superfície e de um comprimento de aresta desejado; não é simplificação nem garante uma contagem final de polígonos, podendo até adicionar triângulos. **Aderência: Alta; o worker offline `mbm-cgal-remesh` agora oferece esse fluxo para OBJ estático.**
- **Polygon Mesh Repair** (reparo de malhas poligonais) — trata defeitos combinatórios e geométricos, como orientação, furos, degenerações e costura de bordas. **Aderência: Alta** para um worker de auditoria e reparo explícito, sempre com relatório e saída separada.
- **3D Surface Subdivision Methods** (subdivisão de superfícies 3D) — refina malhas de controle por esquemas como Catmull-Clark, Loop e Doo-Sabin, aproximando superfícies suaves. **Aderência: Média** para geração offline de modelos suaves.
- **Triangulated Surface Mesh Segmentation** (segmentação de superfícies trianguladas) — agrupa faces por forma e diâmetro local (SDF), usando corte em grafo. **Aderência: Média** para selecionar regiões e preparar recursos 3D.
- **Triangulated Surface Mesh Simplification** (simplificação de superfícies trianguladas) — reduz faces por colapso de arestas com custos, posicionamentos e restrições configuráveis. **Aderência: Média** para comparação experimental; não substituiria automaticamente o QEM próprio, que entende atributos e dados de deformação da engine.
- **Triangulated Surface Mesh Deformation** (deformação de superfícies trianguladas) — move vértices respeitando restrições posicionais, sem exigir estrutura adicional além da malha. **Aderência: Média** para um editor geométrico offline, não para a animação runtime existente.
- **Triangulated Surface Mesh Parameterization** (parametrização de superfícies trianguladas) — planifica malhas topologicamente equivalentes a um disco por métodos conformes, de preservação de área e outros. **Aderência: Alta** para um futuro gerador de UVs; ainda seria necessário tratar costuras UV, distorção e empacotamento das ilhas.
- **Triangulated Surface Mesh Shortest Paths** (caminhos mínimos em superfícies trianguladas) — calcula geodésicas aproximadas sobre uma malha. **Aderência: Média** para ferramentas offline de distâncias sobre a superfície.
- **Triangulated Surface Mesh Skeletonization** (esqueletização de superfícies trianguladas) — extrai um esqueleto de curvas que representa a forma/topologia de uma malha fechada. **Aderência: Média** para geração de estruturas ou análise de formas; não substitui o sistema skeletal de animação.
- **Triangulated Surface Mesh Approximation** (aproximação de superfícies trianguladas) — agrupa triângulos em regiões e substitui grupos por formas representativas, pelo método Variational Shape Approximation. **Aderência: Alta** para criar níveis de detalhe ou malhas leves a comparar com a ferramenta planar e o QEM.
- **The Heat Method** (método do calor) — aproxima distâncias geodésicas de vários pontos-fonte para todos os vértices da malha. **Aderência: Média** para ferramentas de pintura, propagação ou análise offline.
- **Surface Mesh Topology** (topologia de malhas de superfície) — analisa curvas na superfície, ciclos não contráteis e equivalência por homotopia. **Aderência: Baixa/Média** para análise topológica especializada.

## Processamento de conjuntos de pontos

Ferramentas para dados não ordenados: estimar normais, remover ruído, simplificar, registrar e detectar formas.

- **Approximation of Ridges and Umbilics on Triangulated Surface Meshes** (aproximação de cristas e umbílicos) — detecta features derivadas de curvatura. **Aderência: Baixa/Média** para análise ou preservação de detalhes em malhas orgânicas.
- **Estimation of Local Differential Properties of Point-Sampled Surfaces** (estimativa de propriedades diferenciais locais) — estima normais, tangentes, curvaturas principais e variações locais em pontos ou malhas. **Aderência: Média** para ferramentas de diagnóstico e reconstrução.
- **3D Point Set** (conjunto de pontos 3D) — estrutura de pontos que permite associar propriedades como normais, cores e rótulos. **Aderência: Infraestrutura** para futuras ferramentas de scan.
- **Point Set Processing** (processamento de conjuntos de pontos) — simplifica, remove outliers, suaviza, estima/orienta normais, detecta features e registra nuvens. **Aderência: Alta** se o projeto adicionar um pipeline de scan; caso contrário, baixa prioridade.
- **Shape Detection** (detecção de formas) — encontra primitivas como planos, cilindros e esferas em pontos por Efficient RANSAC ou region growing. **Aderência: Média/Alta** para reconstruir assets arquitetônicos e segmentar scans.
- **Shape Regularization** (regularização de formas) — alinha segmentos, contornos e planos próximos a orientações/condições especificadas. **Aderência: Média** como etapa posterior à detecção de planos.
- **2D Placement of Streamlines** (posicionamento de linhas de fluxo 2D) — distribui linhas de fluxo para visualizar campos vetoriais. **Aderência: Baixa** para a engine atual; pode servir a visualizações técnicas.
- **Classification** (classificação) — atribui rótulos a dados usando atributos e classificadores configuráveis. **Aderência: Baixa/Média** sem pipeline de nuvens de pontos ou GIS.

## Busca e ordenação espacial

Estruturas para consultas espaciais rápidas. Para processamento offline, as mais úteis são árvores AABB e octrees; não implicam que a engine deva trocar seu sistema de colisão.

- **2D Range and Neighbor Search** (busca por região e vizinhos 2D) — consultas de vizinhança e região sobre pontos com estrutura Delaunay. **Aderência: Média** para ferramentas de edição 2D.
- **Interval Skip List** (lista de intervalos) — encontra intervalos contendo um ponto e acelera consultas de interseção com faixas. **Aderência: Baixa.**
- **dD Spatial Searching** (busca espacial em dimensão d) — consultas exatas/aproximadas de vizinhos, intervalos e pontos mais próximos/distantes. **Aderência: Média** para processamento offline.
- **dD Range and Segment Trees** (árvores de intervalo e segmento) — consultas estáticas de janelas/regiões em conjuntos de pontos. **Aderência: Baixa/Média.**
- **Intersecting Sequences of dD Iso-oriented Boxes** (interseções entre sequências de caixas alinhadas aos eixos) — encontra pares de caixas sobrepostas, útil como filtro inicial para procurar interseções geométricas. **Aderência: Alta** para acelerar auditorias offline de malhas.
- **2D and 3D Fast Intersection and Distance Computation** (interseção e distância rápidas 2D/3D) — árvore AABB para consultas de interseção, raio e distância em primitivas geométricas. **Aderência: Alta**; o worker planar já usa AABB para estimar erro entre malhas.
- **Quadtrees, Octrees, and Orthtrees** (quadtrees, octrees e orthtrees) — subdivide recursivamente o espaço 2D/3D para consultas e processamento local. **Aderência: Média** para ferramentas offline, voxelização e consultas espaciais.
- **Spatial Sorting** (ordenação espacial) — ordena pontos para melhorar o desempenho de algoritmos geométricos incrementais. **Aderência: Infraestrutura** para outros workers.

## Otimização geométrica

Algoritmos para encontrar medidas e orientações convenientes de conjuntos de pontos ou formas.

- **Bounding Volumes** (volumes envolventes) — calcula esferas, elipsoides, anéis, faixas e retângulos envolventes. **Aderência: Média/Alta** para metadados e colisores aproximados.
- **Inscribed Areas** (regiões inscritas) — encontra polígonos ou retângulos grandes dentro de conjuntos convexos. **Aderência: Baixa**, possivelmente útil em empacotamento 2D.
- **Optimal Distances** (distâncias ótimas) — calcula distâncias entre envoltórias convexas, largura de conjuntos e pontos mais distantes. **Aderência: Média** para validação geométrica offline.
- **Principal Component Analysis** (análise de componentes principais) — calcula centróide, eixos dominantes, caixas alinhadas e ajustes lineares/planos. **Aderência: Alta** para orientar objetos, estimar planos e gerar caixas de colisão.
- **Optimal Bounding Box** (caixa envolvente orientada ótima) — encontra uma caixa orientada compacta para pontos ou uma malha. **Aderência: Alta** para gerar colisores e melhorar a importação e o posicionamento do ponto de origem dos modelos.

## Interpolação

Estima valores ou coordenadas dentro de uma região a partir de amostras.

- **2D and Surface Function Interpolation** (interpolação de funções 2D e de superfície) — interpola valores amostrados e oferece coordenadas de vizinho natural. **Aderência: Média** para campos de altura, deformação ou dados de imagem.
- **2D Generalized Barycentric Coordinates** (coordenadas baricêntricas generalizadas 2D) — expressa pontos de um polígono por pesos associados aos seus vértices. **Aderência: Média** para deformação de sprites e interpolação em polígonos.
- **3D Generalized Barycentric Coordinates** (coordenadas baricêntricas generalizadas 3D) — pesos para pontos em poliedros simpliciais convexos. Introduzido na CGAL 6.2. **Aderência: Média** para transferir deformações em volumes convexos.

## Biblioteca de suporte

Componentes para conectar os algoritmos e estruturas da CGAL ao código C++ e a bibliotecas como Boost.

- **STL Extensions for CGAL** (extensões STL) — algoritmos e estruturas auxiliares ao estilo STL. **Aderência: Infraestrutura.**
- **CGAL and the Boost Graph Library** (CGAL e Boost Graph Library) — adapta estruturas CGAL aos conceitos de grafo do Boost e vice-versa. **Aderência: Infraestrutura** para algoritmos em malhas.
- **CGAL and Solvers** (CGAL e solucionadores) — interfaces para sistemas lineares, otimização inteira/mista e problemas não lineares. **Aderência: Infraestrutura**; relevante quando um algoritmo depender desses solucionadores.
- **CGAL and Boost Property Maps** (CGAL e mapas de propriedades Boost) — conecta propriedades de elementos geométricos às APIs que esperam property maps. **Aderência: Infraestrutura.**
- **Weight Interface** (interface de pesos) — padroniza pesos analíticos, baricêntricos e regionais em 2D/3D. **Aderência: Baixa/Infraestrutura.**
- **Cone-Based Spanners** (spanners baseados em cones) — constrói grafos esparsos que aproximam distâncias entre pontos, como Yao e Theta graphs. **Aderência: Baixa.**
- **Handles and Circulators** (handles e circuladores) — formas de navegar estruturas com ciclos e referências estáveis. **Aderência: Infraestrutura.**
- **Geometric Object Generators** (geradores de objetos geométricos) — cria dados geométricos sintéticos para testes e benchmarks, inclusive casos degenerados. **Aderência: Alta** para robustez e testes de desempenho dos workers.
- **Profiling tools, Hash Map, Union-find, Modifiers** (perfilamento, hash, union-find e modificadores) — utilitários de tempo/memória e estruturas de dados auxiliares. **Aderência: Infraestrutura.**
- **I/O Streams** (fluxos de entrada e saída) — operadores e formatos para ler/escrever objetos geométricos. **Aderência: Alta** para novos executáveis offline, com atenção à compatibilidade de formatos e atributos.

## Visualização

Ferramentas de visualização disponíveis no ecossistema CGAL; podem ajudar em protótipos, mas não substituem a integração com os editores da engine.

- **CGAL and the Qt Graphics View Framework** (CGAL e Qt Graphics View) — integra objetos e estruturas CGAL à infraestrutura gráfica do Qt 6. **Aderência: Baixa** para mini-mbm, que usa Dear ImGui e backends próprios.
- **CGAL Ipelets** (plugins Ipelets) — cria plugins CGAL para o editor geométrico Ipe. **Aderência: Baixa.**
- **Basic Viewer** (visualizador básico) — exibe estruturas de vários pacotes CGAL em uma janela Qt. **Aderência: Média** para protótipos e depuração rápida de workers, mas não como UI de produto.

## O que parece mais promissor para este projeto

Considerando o processador planar existente, o foco 2D da mini-mbm e a intenção de manter CGAL desacoplada, eu priorizaria experimentos offline pequenos e mensuráveis:

O remalhamento isotrópico foi promovido a primeiro worker novo e já está implementado como ferramenta offline. Ele pode servir ao processamento de malhas da mini-mbm e oferecer um ramo local no mesh3dgen quando o objetivo for regularizar triângulos, sem nova solicitação ao provedor. Nesse projeto, simplificação local é uma operação distinta; o remesh remoto é cobrado e usa a tarefa 3D original. As integrações preservam o original e produzem uma variante independente.

O MVP pode aceitar OBJ estático triangulado; cada projeto adapta seu formato nativo ao contrato de intercâmbio. Deve preservar bordas, features, costuras UV e fronteiras de material, e registrar distribuição/comprimento das arestas, qualidade dos triângulos, contagem, tempo e desvio geométrico estimado. Se exceder o orçamento de faces, pode-se avaliar QEM depois do remesh, sem prometer que o remesher atinja uma contagem exata.

Isso não equivale a retopologia semântica, fluxo de arestas dirigido, geração de quads ou bake de texturas. Não substitui esses recursos quando forem o objetivo do serviço de IA.

1. **Auditoria e reparo explícito de malhas:** Polygon Mesh Processing, Polygon Mesh Repair, 2D and 3D Fast Intersection and Distance Computation (árvore AABB) e testes de autointerseção. Começar por um relatório somente leitura; reparos devem escrever outro arquivo e informar o que foi alterado.
2. **Proxies de colisão para Bullet:** 3D Convex Hulls, Convex Decomposition of Polyhedra, 3D Alpha Wrapping, Bounding Volumes e Optimal Bounding Box. Avaliar custo de colisão, desvio/folga geométrica e número de peças por modelo.
3. **Operações 2D para contornos:** 2D Polygon Repair, 2D Regularized Boolean Set-Operations, 2D Polygon Partitioning, 2D Minkowski Sums e 2D Polyline Simplification. Casos de uso possíveis: caixas de colisão, recortes, expansão de contornos e preparação para malha de imagem.
4. **Benchmark de triangulação 2D:** 2D Conforming Triangulations and Meshes. Comparar com a triangulação atual em silhuetas, furos e relevos reais antes de considerar uma substituição.
5. **UVs e aproximação:** Triangulated Surface Mesh Parameterization e Triangulated Surface Mesh Approximation. São possibilidades futuras, mas exigem definir como preservar costuras UV, materiais e requisitos visuais dos recursos.
6. **Reconstrução de nuvens de pontos:** Point Set Processing, Shape Detection e Polygonal Surface Reconstruction. Só priorizar se houver demanda concreta por importar digitalizações ou dados de fotogrametria.

Não parece prioritário substituir o QEM da engine por um simplificador genérico da CGAL: o QEM existente trata atributos e dados de deformação específicos. Também não há motivo para levar as estruturas CGAL à execução da engine apenas por estarem disponíveis; cada ferramenta deve justificar seu custo de compilação, dependências, licença e manutenção.

## Observações sobre versão e licença

- A página `latest` consultada aponta para CGAL 6.2.1, enquanto o README deste repositório exige CGAL >= 6.0. Pacotes introduzidos em 6.1/6.2 podem exigir uma atualização explícita da dependência.
- As licenças variam entre pacotes e dependências. Antes de distribuir um novo executável, confira a licença do pacote específico, das dependências efetivamente usadas e as obrigações de distribuição aplicáveis. Este projeto identifica os tools combinados atuais como GPL-3.0-or-later.
- Este catálogo é uma síntese para orientação, não substitui os manuais de referência nem uma avaliação de robustez, desempenho ou licença para um caso concreto.
