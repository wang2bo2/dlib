// Copyright (C) 2015  Davis E. King (davis@dlib.net)
// License: Boost Software License   See LICENSE.txt for the full license.
#undef DLIB_DNn_LOSS_ABSTRACT_H_
#ifdef DLIB_DNn_LOSS_ABSTRACT_H_

#include "core_abstract.h"
#include "../image_processing/full_object_detection_abstract.h"

namespace dlib
{

// ----------------------------------------------------------------------------------------

    class EXAMPLE_LOSS_LAYER_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                A loss layer is the final layer in a deep neural network.  It computes the
                task loss.  That is, it computes a number that tells us how well the
                network is performing on some task, such as predicting a binary label.  

                You can use one of the loss layers that comes with dlib (defined below).
                But importantly, you are able to define your own loss layers to suit your
                needs.  You do this by creating a class that defines an interface matching
                the one described by this EXAMPLE_LOSS_LAYER_ class.  Note that there is no
                dlib::EXAMPLE_LOSS_LAYER_ type.  It is shown here purely to document the
                interface that a loss layer must implement.

                A loss layer can optionally provide a to_label() method that converts the
                output of a network into a user defined type.  If to_label() is not
                provided then the operator() methods of add_loss_layer will not be
                available, but otherwise everything will function as normal.

                Finally, note that there are two broad flavors of loss layer, supervised
                and unsupervised.  The EXAMPLE_LOSS_LAYER_ as shown here is a supervised
                layer.  To make an unsupervised loss you simply leave out the
                training_label_type typedef and the truth iterator argument to
                compute_loss_value_and_gradient().
        !*/

    public:

        // In most cases training_label_type and output_label_type will be the same type.
        typedef whatever_type_you_use_for_training_labels training_label_type;
        typedef whatever_type_you_use_for_outout_labels   output_label_type;

        EXAMPLE_LOSS_LAYER_ (
        );
        /*!
            ensures
                - EXAMPLE_LOSS_LAYER_ objects are default constructable.
        !*/

        EXAMPLE_LOSS_LAYER_ (
            const EXAMPLE_LOSS_LAYER_& item
        );
        /*!
            ensures
                - EXAMPLE_LOSS_LAYER_ objects are copy constructable.
        !*/

        // Implementing to_label() is optional.
        template <
            typename SUB_TYPE,
            typename label_iterator
            >
        void to_label (
            const tensor& input_tensor,
            const SUB_TYPE& sub,
            label_iterator iter
        ) const;
        /*!
            requires
                - SUBNET implements the SUBNET interface defined at the top of
                  layers_abstract.h.
                - input_tensor was given as input to the network sub and the outputs are
                  now visible in layer<i>(sub).get_output(), for all valid i.
                - input_tensor.num_samples() > 0
                - input_tensor.num_samples()%sub.sample_expansion_factor() == 0.
                - iter == an iterator pointing to the beginning of a range of
                  input_tensor.num_samples()/sub.sample_expansion_factor() elements.  Moreover,
                  they must be output_label_type elements.
            ensures
                - Converts the output of the provided network to output_label_type objects and
                  stores the results into the range indicated by iter.  In particular, for
                  all valid i, it will be the case that:
                    *(iter+i/sub.sample_expansion_factor()) is populated based on the output of
                    sub and corresponds to the ith sample in input_tensor.
        !*/

        template <
            typename const_label_iterator,
            typename SUBNET
            >
        double compute_loss_value_and_gradient (
            const tensor& input_tensor,
            const_label_iterator truth, 
            SUBNET& sub
        ) const;
        /*!
            requires
                - SUBNET implements the SUBNET interface defined at the top of
                  layers_abstract.h.
                - input_tensor was given as input to the network sub and the outputs are
                  now visible in layer<i>(sub).get_output(), for all valid i.
                - input_tensor.num_samples() > 0
                - input_tensor.num_samples()%sub.sample_expansion_factor() == 0.
                - for all valid i:
                    - layer<i>(sub).get_gradient_input() has the same dimensions as
                      layer<i>(sub).get_output().
                - truth == an iterator pointing to the beginning of a range of
                  input_tensor.num_samples()/sub.sample_expansion_factor() elements.  Moreover,
                  they must be training_label_type elements.
                - for all valid i:
                    - *(truth+i/sub.sample_expansion_factor()) is the label of the ith sample in
                      input_tensor.
            ensures
                - This function computes a loss function that describes how well the output
                  of sub matches the expected labels given by truth.  Let's write the loss
                  function as L(input_tensor, truth, sub).  
                - Then compute_loss_value_and_gradient() computes the gradient of L() with
                  respect to the outputs in sub.  Specifically, compute_loss_value_and_gradient() 
                  assigns the gradients into sub by performing the following tensor
                  assignments, for all valid i: 
                    - layer<i>(sub).get_gradient_input() = the gradient of
                      L(input_tensor,truth,sub) with respect to layer<i>(sub).get_output().
                - returns L(input_tensor,truth,sub)
        !*/
    };

    std::ostream& operator<<(std::ostream& out, const EXAMPLE_LOSS_LAYER_& item);
    /*!
        print a string describing this layer.
    !*/

    void to_xml(const EXAMPLE_LOSS_LAYER_& item, std::ostream& out);
    /*!
        This function is optional, but required if you want to print your networks with
        net_to_xml().  Therefore, to_xml() prints a layer as XML.
    !*/

    void serialize(const EXAMPLE_LOSS_LAYER_& item, std::ostream& out);
    void deserialize(EXAMPLE_LOSS_LAYER_& item, std::istream& in);
    /*!
        provides serialization support  
    !*/

    // For each loss layer you define, always define an add_loss_layer template so that
    // layers can be easily composed.  Moreover, the convention is that the layer class
    // ends with an _ while the add_loss_layer template has the same name but without the
    // trailing _.
    template <typename SUBNET>
    using EXAMPLE_LOSS_LAYER = add_loss_layer<EXAMPLE_LOSS_LAYER_, SUBNET>;

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

    class loss_binary_hinge_ 
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This object implements the loss layer interface defined above by
                EXAMPLE_LOSS_LAYER_.  In particular, it implements the hinge loss, which is
                appropriate for binary classification problems.  Therefore, the possible
                labels when using this loss are +1 and -1.  Moreover, it will cause the
                network to produce outputs > 0 when predicting a member of the +1 class and
                values < 0 otherwise.
        !*/
    public:

        typedef float training_label_type;
        typedef float output_label_type;

        template <
            typename SUB_TYPE,
            typename label_iterator
            >
        void to_label (
            const tensor& input_tensor,
            const SUB_TYPE& sub,
            label_iterator iter
        ) const;
        /*!
            This function has the same interface as EXAMPLE_LOSS_LAYER_::to_label() except
            it has the additional calling requirements that: 
                - sub.get_output().nr() == 1
                - sub.get_output().nc() == 1
                - sub.get_output().k() == 1
                - sub.get_output().num_samples() == input_tensor.num_samples()
                - sub.sample_expansion_factor() == 1
            and the output label is the raw score for each classified object.  If the score
            is > 0 then the classifier is predicting the +1 class, otherwise it is
            predicting the -1 class.
        !*/

        template <
            typename const_label_iterator,
            typename SUBNET
            >
        double compute_loss_value_and_gradient (
            const tensor& input_tensor,
            const_label_iterator truth, 
            SUBNET& sub
        ) const;
        /*!
            This function has the same interface as EXAMPLE_LOSS_LAYER_::compute_loss_value_and_gradient() 
            except it has the additional calling requirements that: 
                - sub.get_output().nr() == 1
                - sub.get_output().nc() == 1
                - sub.get_output().k() == 1
                - sub.get_output().num_samples() == input_tensor.num_samples()
                - sub.sample_expansion_factor() == 1
                - all values pointed to by truth are +1 or -1.
        !*/

    };

    template <typename SUBNET>
    using loss_binary_hinge = add_loss_layer<loss_binary_hinge_, SUBNET>;

// ----------------------------------------------------------------------------------------

    class loss_binary_log_ 
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This object implements the loss layer interface defined above by
                EXAMPLE_LOSS_LAYER_.  In particular, it implements the log loss, which is
                appropriate for binary classification problems.  Therefore, the possible
                labels when using this loss are +1 and -1.  Moreover, it will cause the
                network to produce outputs > 0 when predicting a member of the +1 class and
                values < 0 otherwise.

                To be more specific, this object contains a sigmoid layer followed by a 
                cross-entropy layer.  
        !*/
    public:

        typedef float training_label_type;
        typedef float output_label_type;

        template <
            typename SUB_TYPE,
            typename label_iterator
            >
        void to_label (
            const tensor& input_tensor,
            const SUB_TYPE& sub,
            label_iterator iter
        ) const;
        /*!
            This function has the same interface as EXAMPLE_LOSS_LAYER_::to_label() except
            it has the additional calling requirements that: 
                - sub.get_output().nr() == 1
                - sub.get_output().nc() == 1
                - sub.get_output().k() == 1
                - sub.get_output().num_samples() == input_tensor.num_samples()
                - sub.sample_expansion_factor() == 1
            and the output label is the raw score for each classified object.  If the score
            is > 0 then the classifier is predicting the +1 class, otherwise it is
            predicting the -1 class.
        !*/

        template <
            typename const_label_iterator,
            typename SUBNET
            >
        double compute_loss_value_and_gradient (
            const tensor& input_tensor,
            const_label_iterator truth, 
            SUBNET& sub
        ) const;
        /*!
            This function has the same interface as EXAMPLE_LOSS_LAYER_::compute_loss_value_and_gradient() 
            except it has the additional calling requirements that: 
                - sub.get_output().nr() == 1
                - sub.get_output().nc() == 1
                - sub.get_output().k() == 1
                - sub.get_output().num_samples() == input_tensor.num_samples()
                - sub.sample_expansion_factor() == 1
                - all values pointed to by truth are +1 or -1.
        !*/

    };

    template <typename SUBNET>
    using loss_binary_log = add_loss_layer<loss_binary_log_, SUBNET>;

// ----------------------------------------------------------------------------------------

    class loss_multiclass_log_ 
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This object implements the loss layer interface defined above by
                EXAMPLE_LOSS_LAYER_.  In particular, it implements the multiclass logistic
                regression loss (e.g. negative log-likelihood loss), which is appropriate
                for multiclass classification problems.  This means that the possible
                labels when using this loss are integers >= 0.  
                
                Moreover, if after training you were to replace the loss layer of the
                network with a softmax layer, the network outputs would give the
                probabilities of each class assignment.  That is, if you have K classes
                then the network should output tensors with the tensor::k()'th dimension
                equal to K.  Applying softmax to these K values gives the probabilities of
                each class.  The index into that K dimensional vector with the highest
                probability is the predicted class label.
        !*/

    public:

        typedef unsigned long training_label_type;
        typedef unsigned long output_label_type;

        template <
            typename SUB_TYPE,
            typename label_iterator
            >
        void to_label (
            const tensor& input_tensor,
            const SUB_TYPE& sub,
            label_iterator iter
        ) const;
        /*!
            This function has the same interface as EXAMPLE_LOSS_LAYER_::to_label() except
            it has the additional calling requirements that: 
                - sub.get_output().nr() == 1
                - sub.get_output().nc() == 1
                - sub.get_output().num_samples() == input_tensor.num_samples()
                - sub.sample_expansion_factor() == 1
            and the output label is the predicted class for each classified object.  The number
            of possible output classes is sub.get_output().k().
        !*/

        template <
            typename const_label_iterator,
            typename SUBNET
            >
        double compute_loss_value_and_gradient (
            const tensor& input_tensor,
            const_label_iterator truth, 
            SUBNET& sub
        ) const;
        /*!
            This function has the same interface as EXAMPLE_LOSS_LAYER_::compute_loss_value_and_gradient() 
            except it has the additional calling requirements that: 
                - sub.get_output().nr() == 1
                - sub.get_output().nc() == 1
                - sub.get_output().num_samples() == input_tensor.num_samples()
                - sub.sample_expansion_factor() == 1
                - all values pointed to by truth are < sub.get_output().k()
        !*/

    };

    template <typename SUBNET>
    using loss_multiclass_log = add_loss_layer<loss_multiclass_log_, SUBNET>;

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

    struct mmod_options
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This object contains all the parameters that control the behavior of loss_mmod_.
        !*/

    public:

        struct detector_window_size
        {
            detector_window_size() = default; 
            detector_window_size(unsigned long w, unsigned long h) : width(w), height(h) {}

            unsigned long width = 0;
            unsigned long height = 0;

            friend inline void serialize(const detector_window_size& item, std::ostream& out);
            friend inline void deserialize(detector_window_size& item, std::istream& in);
        };

        mmod_options() = default;

        // This kind of object detector is a sliding window detector.  The detector_windows
        // field determines how many sliding windows we will use and what the shape of each
        // window is.  Since you will usually use the MMOD loss with an image pyramid, the
        // detector sizes also determine the size of the smallest object you can detect.
        std::vector<detector_window_size> detector_windows;

        // These parameters control how we penalize different kinds of mistakes.  See 
        // Max-Margin Object Detection by Davis E. King (http://arxiv.org/abs/1502.00046)
        // for further details.
        double loss_per_false_alarm = 1;
        double loss_per_missed_target = 1;

        // A detection must have an intersection-over-union value greater than this for us
        // to consider it a match against a ground truth box.
        double truth_match_iou_threshold = 0.5;

        // When doing non-max suppression, we use overlaps_nms to decide if a box overlaps
        // an already output detection and should therefore be thrown out.
        test_box_overlap overlaps_nms = test_box_overlap(0.4);

        // Any mmod_rect in the training data that has its ignore field set to true defines
        // an "ignore zone" in an image.  Any detection from that area is totally ignored
        // by the optimizer.  Therefore, this overlaps_ignore field defines how we decide
        // if a box falls into an ignore zone.  You use these ignore zones if there are
        // objects in your dataset that you are unsure if you want to detect or otherwise
        // don't care if the detector gets them or not.  
        test_box_overlap overlaps_ignore;

        mmod_options (
            const std::vector<std::vector<mmod_rect>>& boxes,
            const unsigned long target_size,      
            const unsigned long min_target_size,   
            const double min_detector_window_overlap_iou = 0.75
        );
        /*!
            requires
                - 0 < min_target_size <= target_size
                - 0.5 < min_detector_window_overlap_iou < 1
            ensures
                - This function tries to automatically set the MMOD options to reasonable
                  values, assuming you have a training dataset of boxes.size() images, where
                  the ith image contains objects boxes[i] you want to detect.
                - The most important thing this function does is decide what detector
                  windows should be used.  This is done by finding a set of detector
                  windows that are sized such that:
                    - When slid over an image pyramid, each box in boxes will have an
                      intersection-over-union with one of the detector windows of at least
                      min_detector_window_overlap_iou.  That is, we will make sure that
                      each box in boxes could potentially be detected by one of the
                      detector windows.  This essentially comes down to picking detector
                      windows with aspect ratios similar to the aspect ratios in boxes.
                    - The longest edge of each detector window is target_size pixels in
                      length, unless the window's shortest side would be less than
                      min_target_size pixels in length.  In this case the shortest side
                      will be set to min_target_size length, and the other side sized to
                      preserve the aspect ratio of the window.  
                  This means that, target_size and min_target_size control the size of the
                  detector windows, while the aspect ratios of the detector windows are
                  automatically determined by the contents of boxes.  It should also be
                  emphasized that the detector isn't going to be able to detect objects
                  smaller than any of the detector windows.  So consider that when setting
                  these sizes.
                - This function will also set the overlaps_nms tester to the most
                  restrictive tester that doesn't reject anything in boxes.
        !*/
    };

    void serialize(const mmod_options& item, std::ostream& out);
    void deserialize(mmod_options& item, std::istream& in);

// ----------------------------------------------------------------------------------------

    class loss_mmod_ 
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This object implements the loss layer interface defined above by
                EXAMPLE_LOSS_LAYER_.  In particular, it implements the Max Margin Object
                Detection loss defined in the paper:
                    Max-Margin Object Detection by Davis E. King (http://arxiv.org/abs/1502.00046).
               
                This means you use this loss if you want to detect the locations of objects
                in images.

                It should also be noted that this loss layer requires an input layer that
                defines the following functions:
                    - image_contained_point()
                    - tensor_space_to_image_space()
                    - image_space_to_tensor_space()
                A reference implementation of them and their definitions can be found in
                the input_rgb_image_pyramid object, which is the recommended input layer to
                be used with loss_mmod_.
        !*/

    public:

        typedef std::vector<mmod_rect> training_label_type;
        typedef std::vector<mmod_rect> output_label_type;

        loss_mmod_(
        );
        /*!
            ensures
                - #get_options() == mmod_options()
        !*/

        loss_mmod_(
            mmod_options options_
        );
        /*!
            ensures
                - #get_options() == options_
        !*/

        const mmod_options& get_options (
        ) const;
        /*!
            ensures
                - returns the options object that defines the general behavior of this loss layer.
        !*/

        template <
            typename SUB_TYPE,
            typename label_iterator
            >
        void to_label (
            const tensor& input_tensor,
            const SUB_TYPE& sub,
            label_iterator iter,
            double adjust_threshold = 0
        ) const;
        /*!
            This function has the same interface as EXAMPLE_LOSS_LAYER_::to_label() except
            it has the additional calling requirements that: 
                - sub.get_output().k() == 1
                - sub.get_output().num_samples() == input_tensor.num_samples()
                - sub.sample_expansion_factor() == 1
            Also, the output labels are std::vectors of mmod_rects where, for each mmod_rect R,
            we have the following interpretations:
                - R.rect == the location of an object in the image.
                - R.detection_confidence the score for the object, the bigger the score the
                  more confident the detector is that an object is really there.  Only
                  objects with a detection_confidence > adjust_threshold are output.  So if
                  you want to output more objects (that are also of less confidence) you
                  can call to_label() with a smaller value of adjust_threshold.
                - R.ignore == false (this value is unused by to_label()).
        !*/

        template <
            typename const_label_iterator,
            typename SUBNET
            >
        double compute_loss_value_and_gradient (
            const tensor& input_tensor,
            const_label_iterator truth, 
            SUBNET& sub
        ) const;
        /*!
            This function has the same interface as EXAMPLE_LOSS_LAYER_::compute_loss_value_and_gradient() 
            except it has the additional calling requirements that: 
                - sub.get_output().k() == 1
                - sub.get_output().num_samples() == input_tensor.num_samples()
                - sub.sample_expansion_factor() == 1
            Also, the loss value returned is roughly equal to the average number of
            mistakes made per image.  This is the sum of false alarms and missed
            detections, weighted by the loss weights for these types of mistakes specified
            in the mmod_options.
        !*/
    };

    template <typename SUBNET>
    using loss_mmod = add_loss_layer<loss_mmod_, SUBNET>;

// ----------------------------------------------------------------------------------------

    class loss_metric_ 
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This object implements the loss layer interface defined above by
                EXAMPLE_LOSS_LAYER_.  In particular, it allows you to learn to map objects
                into a vector space where objects sharing the same class label are close to
                each other, while objects with different labels are far apart.   

                To be specific, it optimizes the following loss function which considers
                all pairs of objects in a mini-batch and computes a different loss depending 
                on their respective class labels.  So if objects A1 and A2 in a mini-batch
                share the same class label then their contribution to the loss is:
                    max(0, length(A1-A2)-get_distance_threshold() + get_margin())

                While if A1 and B1 have different class labels then their contribution to
                the loss function is:
                    max(0, get_distance_threshold()-length(A1-B1) + get_margin())

                Therefore, this loss layer optimizes a version of the hinge loss.
                Moreover, the loss is trying to make sure that all objects with the same
                label are within get_distance_threshold() distance of each other.
                Conversely, if two objects have different labels then they should be more
                than get_distance_threshold() distance from each other in the learned
                embedding.  So this loss function gives you a natural decision boundary for
                deciding if two objects are from the same class.

                Finally, the loss balances the number of negative pairs relative to the
                number of positive pairs.  Therefore, if there are N pairs that share the
                same identity in a mini-batch then the algorithm will only include the N
                worst non-matching pairs in the loss.  That is, the algorithm performs hard
                negative mining on the non-matching pairs.  This is important since there
                are in general way more non-matching pairs than matching pairs.  So to
                avoid imbalance in the loss this kind of hard negative mining is useful.
        !*/
    public:

        typedef unsigned long training_label_type;
        typedef matrix<float,0,1> output_label_type;

        loss_metric_(
        );
        /*!
            ensures
                - #get_margin() == 0.04
                - #get_distance_threshold() == 0.6
        !*/

        loss_metric_(
            float margin,
            float dist_thresh
        );
        /*!
            requires
                - margin > 0
                - dist_thresh > 0
            ensures
                - #get_margin() == margin
                - #get_distance_threshold() == dist_thresh
        !*/

        template <
            typename SUB_TYPE,
            typename label_iterator
            >
        void to_label (
            const tensor& input_tensor,
            const SUB_TYPE& sub,
            label_iterator iter
        ) const;
        /*!
            This function has the same interface as EXAMPLE_LOSS_LAYER_::to_label() except
            it has the additional calling requirements that: 
                - sub.get_output().nr() == 1
                - sub.get_output().nc() == 1
                - sub.get_output().num_samples() == input_tensor.num_samples()
                - sub.sample_expansion_factor() == 1
            This loss expects the network to produce a single vector (per sample) as
            output.  This vector is the learned embedding.  Therefore, to_label() just
            copies these output vectors from the network into the output label_iterators
            given to this function, one for each sample in the input_tensor.
        !*/

        float get_margin() const; 
        /*!
            ensures
                - returns the margin value used by the loss function.  See the discussion
                  in WHAT THIS OBJECT REPRESENTS for details.
        !*/

        float get_distance_threshold() const; 
        /*!
            ensures
                - returns the distance threshold value used by the loss function.  See the discussion
                  in WHAT THIS OBJECT REPRESENTS for details.
        !*/

        template <
            typename const_label_iterator,
            typename SUBNET
            >
        double compute_loss_value_and_gradient (
            const tensor& input_tensor,
            const_label_iterator truth, 
            SUBNET& sub
        ) const;
        /*!
            This function has the same interface as EXAMPLE_LOSS_LAYER_::compute_loss_value_and_gradient() 
            except it has the additional calling requirements that: 
                - sub.get_output().nr() == 1
                - sub.get_output().nc() == 1
                - sub.get_output().num_samples() == input_tensor.num_samples()
                - sub.sample_expansion_factor() == 1
        !*/

    };

    template <typename SUBNET>
    using loss_metric = add_loss_layer<loss_metric_, SUBNET>;

// ----------------------------------------------------------------------------------------

    class loss_mean_squared_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This object implements the loss layer interface defined above by
                EXAMPLE_LOSS_LAYER_.  In particular, it implements the mean squared loss, which is
                appropriate for regression problems.
        !*/
    public:

        typedef float training_label_type;
        typedef float output_label_type;

        template <
            typename SUB_TYPE,
            typename label_iterator
            >
        void to_label (
            const tensor& input_tensor,
            const SUB_TYPE& sub,
            label_iterator iter
        ) const;
        /*!
            This function has the same interface as EXAMPLE_LOSS_LAYER_::to_label() except
            it has the additional calling requirements that:
                - sub.get_output().nr() == 1
                - sub.get_output().nc() == 1
                - sub.get_output().k() == 1
                - sub.get_output().num_samples() == input_tensor.num_samples()
                - sub.sample_expansion_factor() == 1
            and the output label is the predicted continuous variable.
        !*/

        template <
            typename const_label_iterator,
            typename SUBNET
            >
        double compute_loss_value_and_gradient (
            const tensor& input_tensor,
            const_label_iterator truth,
            SUBNET& sub
        ) const;
        /*!
            This function has the same interface as EXAMPLE_LOSS_LAYER_::compute_loss_value_and_gradient()
            except it has the additional calling requirements that:
                - sub.get_output().nr() == 1
                - sub.get_output().nc() == 1
                - sub.get_output().k() == 1
                - sub.get_output().num_samples() == input_tensor.num_samples()
                - sub.sample_expansion_factor() == 1
        !*/

    };

    template <typename SUBNET>
    using loss_mean_squared = add_loss_layer<loss_mean_squared_, SUBNET>;

// ----------------------------------------------------------------------------------------

    class loss_mean_squared_multioutput_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This object implements the loss layer interface defined above by
                EXAMPLE_LOSS_LAYER_.  In particular, it implements the mean squared loss,
                which is appropriate for regression problems.  It is basically just like
                loss_mean_squared_ except that it lets you define multiple outputs instead
                of just 1.
        !*/
    public:

        typedef matrix<float> training_label_type;
        typedef matrix<float> output_label_type;

        template <
            typename SUB_TYPE,
            typename label_iterator
            >
        void to_label (
            const tensor& input_tensor,
            const SUB_TYPE& sub,
            label_iterator iter
        ) const;
        /*!
            This function has the same interface as EXAMPLE_LOSS_LAYER_::to_label() except
            it has the additional calling requirements that:
                - sub.get_output().nr() == 1
                - sub.get_output().nc() == 1
                - sub.get_output().num_samples() == input_tensor.num_samples()
                - sub.sample_expansion_factor() == 1
            and the output label is the predicted continuous variable.
        !*/

        template <
            typename const_label_iterator,
            typename SUBNET
            >
        double compute_loss_value_and_gradient (
            const tensor& input_tensor,
            const_label_iterator truth,
            SUBNET& sub
        ) const;
        /*!
            This function has the same interface as EXAMPLE_LOSS_LAYER_::compute_loss_value_and_gradient()
            except it has the additional calling requirements that:
                - sub.get_output().nr() == 1
                - sub.get_output().nc() == 1
                - sub.get_output().num_samples() == input_tensor.num_samples()
                - sub.sample_expansion_factor() == 1
                - (*(truth + idx)).nc() == 1 for all idx such that 0 <= idx < sub.get_output().num_samples()
                - (*(truth + idx)).nr() == sub.get_output().k() for all idx such that 0 <= idx < sub.get_output().num_samples()
        !*/

    };

    template <typename SUBNET>
    using loss_mean_squared_multioutput = add_loss_layer<loss_mean_squared_multioutput_, SUBNET>;

// ----------------------------------------------------------------------------------------

    class loss_multiclass_log_per_pixel_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This object implements the loss layer interface defined above by
                EXAMPLE_LOSS_LAYER_.  In particular, it implements the multiclass logistic
                regression loss (e.g. negative log-likelihood loss), which is appropriate
                for multiclass classification problems.  It is basically just like
                loss_multiclass_log_ except that it lets you define matrix outputs instead
                of scalar outputs.  It should be useful, for example, in semantic
                segmentation where we want to classify each pixel of an image.
        !*/
    public:

        // In semantic segmentation, if you don't know the ground-truth of some pixel,
        // set the label of that pixel to this value. When you do so, the pixel will be
        // ignored when computing gradients.
        static const uint16_t label_to_ignore = std::numeric_limits<uint16_t>::max();

        // In semantic segmentation, 65535 classes ought to be enough for anybody.
        typedef matrix<uint16_t> training_label_type;
        typedef matrix<uint16_t> output_label_type;

        template <
            typename SUB_TYPE,
            typename label_iterator
            >
        void to_label (
            const tensor& input_tensor,
            const SUB_TYPE& sub,
            label_iterator iter
        ) const;
        /*!
            This function has the same interface as EXAMPLE_LOSS_LAYER_::to_label() except
            it has the additional calling requirements that:
                - sub.get_output().num_samples() == input_tensor.num_samples()
                - sub.sample_expansion_factor() == 1
            and the output label is the predicted class for each classified element.  The number
            of possible output classes is sub.get_output().k().
        !*/

        template <
            typename const_label_iterator,
            typename SUBNET
            >
        double compute_loss_value_and_gradient (
            const tensor& input_tensor,
            const_label_iterator truth,
            SUBNET& sub
        ) const;
        /*!
            This function has the same interface as EXAMPLE_LOSS_LAYER_::compute_loss_value_and_gradient()
            except it has the additional calling requirements that:
                - sub.get_output().num_samples() == input_tensor.num_samples()
                - sub.sample_expansion_factor() == 1
                - all values pointed to by truth are < sub.get_output().k() or are equal to label_to_ignore.
        !*/

    };

    template <typename SUBNET>
    using loss_multiclass_log_per_pixel = add_loss_layer<loss_multiclass_log_per_pixel_, SUBNET>;

// ----------------------------------------------------------------------------------------

    class loss_multiclass_log_per_pixel_weighted_
    {
        /*!
            WHAT THIS OBJECT REPRESENTS
                This object implements the loss layer interface defined above by
                EXAMPLE_LOSS_LAYER_.  In particular, it implements the multiclass logistic
                regression loss (e.g. negative log-likelihood loss), which is appropriate
                for multiclass classification problems.  It is basically just like
                loss_multiclass_log_per_pixel_ except that it lets you define per-pixel
                weights, which may be useful e.g. if you want to emphasize rare classes
                while training.  (If the classification problem is difficult, a flat weight
                structure may lead the network to always predict the most common label, in
                particular if the degree of imbalance is high.  To emphasize a certain
                class or classes, simply increase the weights of the corresponding pixels,
                relative to the weights of the other pixels.)

                Note that if you set the weight to 0 whenever a pixel's label is equal to
                loss_multiclass_log_per_pixel_::label_to_ignore, and to 1 otherwise, then
                you essentially get loss_multiclass_log_per_pixel_ as a special case.
        !*/
    public:

        struct weighted_label
        {
            /*!
                WHAT THIS OBJECT REPRESENTS
                    This object represents the truth label of a single pixel, together with
                    an associated weight (the higher the weight, the more emphasis the
                    corresponding pixel is given during the training).
            !*/

            weighted_label();
            weighted_label(uint16_t label, float weight = 1.f);

            // The ground-truth label. In semantic segmentation, 65536 classes ought to be
            // enough for anybody.
            uint16_t label = 0;

            // The weight of the corresponding pixel.
            float weight = 1.f;
        };

        typedef matrix<weighted_label> training_label_type;
        typedef matrix<uint16_t> output_label_type;

        template <
            typename SUB_TYPE,
            typename label_iterator
            >
        void to_label (
            const tensor& input_tensor,
            const SUB_TYPE& sub,
            label_iterator iter
        ) const;
        /*!
            This function has the same interface as EXAMPLE_LOSS_LAYER_::to_label() except
            it has the additional calling requirements that:
                - sub.get_output().num_samples() == input_tensor.num_samples()
                - sub.sample_expansion_factor() == 1
            and the output label is the predicted class for each classified element.  The number
            of possible output classes is sub.get_output().k().
        !*/

        template <
            typename const_label_iterator,
            typename SUBNET
            >
        double compute_loss_value_and_gradient (
            const tensor& input_tensor,
            const_label_iterator truth,
            SUBNET& sub
        ) const;
        /*!
            This function has the same interface as EXAMPLE_LOSS_LAYER_::compute_loss_value_and_gradient()
            except it has the additional calling requirements that:
                - sub.get_output().num_samples() == input_tensor.num_samples()
                - sub.sample_expansion_factor() == 1
                - all labels pointed to by truth are < sub.get_output().k(), or the corresponding weight
                  is zero.
        !*/

    };

    template <typename SUBNET>
    using loss_multiclass_log_per_pixel_weighted = add_loss_layer<loss_multiclass_log_per_pixel_weighted_, SUBNET>;

// ----------------------------------------------------------------------------------------

}

#endif // DLIB_DNn_LOSS_ABSTRACT_H_

