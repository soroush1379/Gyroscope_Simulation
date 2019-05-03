function [p, z] = makeDGPBiswasYe(graph, anchors, distances, N)
%MAKEDGPBISWASYE Creates a Distance Geometry Problem quadratic polynomial.
%   Creates a Distance Geometry Problem quartic polynomial given a graph
%   and anchors.

E = graph;
A = anchors;
D = distances;

num_anchors = size(A(:,all(~isnan(A))),2);
dim = size(A,1); 
n = N - num_anchors; 

C = [zeros(n,1), eye(n), zeros(n,1)]; % vertex selection vectors
Mpq = {};

% Create selection matrices for node-node distances
for ind_e = 1 : size(E,1)
    
    ind_p = E(ind_e,1);
    ind_q = E(ind_e,2);
    
    both_vertices = (ind_p > 1 && ind_p < N) && (ind_q > 1 && ind_q < N); % not anchors
    
    if both_vertices
        ep = C(:,ind_p)';
        eq = C(:,ind_q)';
        
        Epq = (ep - eq)'*(ep - eq);
        
        Mpq_ = zeros(n + dim);
        Mpq_(dim+1:end, dim+1:end) = Epq;
        Mpq{end + 1} = Mpq_;
        
        continue;
    end
    
    p_anchor = (ind_p == 1 || ind_p > N-1 ) && (ind_q > 1 && ind_q < N);
    
    if p_anchor
        eq = C(:,ind_q)';
        a = A(:,ind_p);
        
        Mpq{end + 1} = [a', -eq]'*[a', -eq];
                
        continue;
    end
    
    q_anchor = (ind_q == 1 || ind_q > N-1 ) && (ind_p > 1 && ind_p < N);
    
    if q_anchor
        ep = C(:,ind_p)';
        a = A(:,ind_q);
        
        Mpq{end + 1} = [-a', ep]'*[-a', ep];
        
        continue;
    end
    
end

%% Expand the polynomial
X = sym('x', [dim,n]); % create symbolic position matrix
assume(X, 'real'); % assume real coefficients
x = reshape(X, dim*n, 1);

Y = sym('y', [n,n]); % create symbolic position matrix
assume(Y, 'real');
Y = triu(Y) + triu(Y,1)';

Z = [eye(dim), X; X', Y];

p = 0;

for ind = 1:size(Mpq,2)
    p = p + (trace(Mpq{ind}*Z) - D(ind, ind+1)^2)^2; %create polynomial
end

upperTriangleIndices = triu(true(n));
z = [x; Y(upperTriangleIndices)];

end

