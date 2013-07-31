function collect =  attack_test(options)

    base_dir = options.base_dir;
    tmp_dir = options.tmp_dir;
    embed_level = options.embed_level;
    seed = options.seed;
    im_dim = options.im_dim;
    dwrs = options.dwrs;
    or = options.orientation;
    attacktype = options.attacktype;
    jpeg_quality = options.jpeg_rate;
    j2k_rate = options.j2k_rate;
    num_random = options.num_random;
    detectortype = options.detectortype;
    ggdshape = options.ggdshape;
    ggd_est_fit = options.est_fit;

    rand('twister',seed);
    W = sign(rand(im_dim,im_dim,1)-0.5);

    h0 = 1; 
    h1 = 2;

    kdu_compress = 'kdu_compress';
    kdu_expand = 'kdu_expand';


%% Load images (stored in base_dir, grayscale)
    listing = dir(fullfile(base_dir,'*.pgm'));
    nimages = size(listing,1);
    cnt = 1;
    for i=1:nimages
        im = imread(fullfile(base_dir,listing(i).name));
        images{cnt} = im;
        imagename{cnt} = listing(i).name;
        cnt=cnt+1;
    end
    
%% Experimental Section
    for image=1:length(images)
        fprintf('Embedding watermark for tests in image: %s\n',imagename{image});
        for dwr=1:length(dwrs)
            [db,hostwm] = wm_embed(images{image}, W,embed_level,'near_sym_b','qshift_b',dwrs(dwr),embed_level,or);
            collect{h0,image,dwr}.watermarked = hostwm;
            collect{h0,image,dwr}.original = images{image};
            collect{h0,image,dwr}.db = db;
            collect{h0,image,dwr}.options = options;
        end
    end

%% Examining H1

    WA = sign(rand(im_dim,im_dim,num_random)-0.5);
    for image=1:length(images)
        for dwr=1:length(dwrs)
            sdr = []; attackpsnr = [];
            for i=1:num_random
                [b,hostwm] = wm_embed(images{image}, WA(:,:,i),embed_level,'near_sym_b','qshift_b',dwrs(dwr),embed_level,or);
                [MSE_ba,PSNR_ba,AD_ba,SC_ba,NK_ba,MD_ba,LMSE_ba,NAE_ba] = iq_measures(images{image},double(hostwm));
                collect{h1,image,dwr}.db_ba = PSNR_ba;
                if (attacktype)
                    switch attacktype
                        case 1
                            attackfilename = sprintf('%s/%s-%d-%d.jpg',tmp_dir, ...
                                imagename{image}, ...
                                dwrs(dwr), ...
                                jpeg_quality);
                            imwrite(hostwm,attackfilename,'Quality',jpeg_quality);
                            attackfile = imread(attackfilename);
                            hostwm = attackfile; % hostwm is now attacked
                            [MSE_aa,PSNR_aa,AD_aa,SC_aa,NK_aa,MD_aa,LMSE_aa,NAE_aa] = iq_measures(images{image},double(hostwm));
                            attackpsnr(i) = PSNR_aa;
                        case 2
                            hosttowrite = sprintf('%s/%s-%d-%0.2f.pgm',tmp_dir,imagename{image},dwrs(dwr),j2k_rate); % write image temporary to disk
                            imwrite(hostwm,hosttowrite,'pgm');
                            intermedfile = sprintf('%s/%s-%d-%0.2f.j2c',tmp_dir,imagename{image},dwrs(dwr),j2k_rate); % intermediate file
                            compress = sprintf('%s -quiet -i %s -o %s -rate %0.2f', ...
                                kdu_compress, hosttowrite, intermedfile, j2k_rate);
                            system(compress);
                            attackfilename = sprintf('%s/%s-%d-%0.2f_attacked.pgm',tmp_dir,imagename{image},dwrs(dwr),j2k_rate); 
                            expand = sprintf('%s -quiet -i %s -o %s', ...
                                kdu_expand, intermedfile, attackfilename);
                            system(expand);
                            attackfile = imread(attackfilename);
                            hostwm = attackfile;
                            [MSE_aa,PSNR_aa,AD_aa,SC_aa,NK_aa,MD_aa,LMSE_aa,NAE_aa] = iq_measures(images{image},double(hostwm));
                            attackpsnr(i) = PSNR_aa;
                    end
                end
                collect{h1,image,dwr}.db = mean(attackpsnr);
                switch detectortype
                    case 'rao'
                        stat(i) = wm_detect_rao(hostwm,WA(:,:,i),embed_level,'near_sym_b','qshift_b',or,ggdshape, ggd_est_fit);
                    case 'lc'
                        stat(i) = wm_detect_lc(hostwm,WA(:,:,i),embed_level,'near_sym_b','qshift_b',or);       
                    case 'hernandez'
                        stat(i) = wm_detect_ggdclassic(hostwm,dwrs(dwr),WA(:,:,i),embed_level,'near_sym_b','qshift_b',or,ggdshape, ggd_est_fit);     
                end
                %%fprintf('L=%0.6f\n',stat(i));
                
            end
            collect{h1,image,dwr}.stat = stat';
        end
    end

%% Examining H0

    for image=1:length(images)
        for dwr=1:length(dwrs)
            if (attacktype)
                switch attacktype
                    case 1
                        attackfilename = sprintf('%s/%s-%d-%d.jpg',tmp_dir, ...
                            imagename{image}, ...
                            dwrs(dwr), ...
                            jpeg_quality);
                        imwrite(collect{h0,image,dwr}.watermarked,attackfilename,'Quality',jpeg_quality);
                        attackfile = imread(attackfilename);
                        hostwm = attackfile;
                    case 2
                        hosttowrite = sprintf('%s/%s-%d-%0.2f.pgm',tmp_dir,imagename{image},dwrs(dwr),j2k_rate);
                        imwrite(collect{h0,image,dwr}.watermarked,hosttowrite,'pgm');
                        intermedfile = sprintf('%s/%s-%d-%0.2f.j2c',tmp_dir,imagename{image},dwrs(dwr),j2k_rate); % intermediate file
                        compress = sprintf('%s -quiet -i %s -o %s -rate %0.2f', ...
                            kdu_compress, hosttowrite, intermedfile, j2k_rate);
                        system(compress);
                        attackfilename = sprintf('%s/%s-%d-%0.2f_attacked.pgm',tmp_dir,imagename{image},dwrs(dwr),j2k_rate); 
                        expand = sprintf('%s -quiet -i %s -o %s', ...
                            kdu_expand, intermedfile, attackfilename);
                        system(expand);
                        attackfile = imread(attackfilename);
                        hostwm = attackfile;
                end
            else
                % set hostwm to image watermarked with W
                hostwm = collect{h0,image,dwr}.watermarked;
            end
            for i=1:num_random
                switch detectortype
                    case 'rao'
                        stat(i) = wm_detect_rao(hostwm,WA(:,:,i),embed_level,'near_sym_b','qshift_b',or,ggdshape, ggd_est_fit);
                    case 'lc'
                        stat(i) = wm_detect_lc(hostwm,WA(:,:,i),embed_level,'near_sym_b','qshift_b',or);       
                    case 'hernandez'
                        stat(i) = wm_detect_ggdclassic(hostwm,dwrs(dwr),WA(:,:,i),embed_level,'near_sym_b','qshift_b',or,ggdshape, ggd_est_fit);     
                end
                %% fprintf('L=%0.6f\n',stat(i));
            end
            collect{h0,image,dwr}.stat = stat';
        end
    end












