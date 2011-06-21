#ifndef __mftp_automaton_hpp__
#define	__mftp_automaton_hpp__

#include "file.hpp"

#include <cstdlib>
#include <queue>
#include <vector>
#include <string.h>
#include <math.h>

#define REQUEST 0u
#define FRAGMENT 1u

struct fragment {
    fileID fid;
    unsigned int offset;
    unsigned char data[FRAGMENT_SIZE];
};

struct request {
    fileID fid;
    unsigned int offset;
    unsigned int request_length;
};

struct message_header {
    uint32_t message_type;
};

struct message {
    message_header header;
    union {
        fragment frag;
        request req;
    };
};

class mftp_automaton :
    public ioa::automaton
{
private:
    std::queue<message> my_req;
    std::vector<bool> their_req;
    std::vector<bool> have;
    std::vector<bool> valid;
    std::queue<message> sendq;
    File m_file;
    bool can_send;

    bool send_precondition () const {
        return (ioa::bind_count (&mftp_automaton::send) != 0) && !sendq.empty() && can_send;
    }

    message send_effect () {
        message m = sendq.front();
        sendq.pop();
        can_send = false;
        return m;
    }

    void receive_effect (const message& m) {
        if (m.header.message_type == FRAGMENT) {
            uint32_t idx = (m.frag.offset / FRAGMENT_SIZE);
            if (!have[idx]) {
                if((m.frag.fid.original_length - m.frag.offset) % FRAGMENT_SIZE == 0) {
                    memcpy(m_file.get_data_ptr() + m.frag.offset, m.frag.data, FRAGMENT_SIZE);
                }
                else {
                    memcpy(m_file.get_data_ptr() + m.frag.offset, m.frag.data, (m.frag.fid.original_length - m.frag.offset));
                }
                have[idx] = true;
                //validate
            }
            their_req[idx] = false;
        }
        else {
            uint32_t idy = (m.req.offset / FRAGMENT_SIZE);
            uint32_t num_frags = ceil (static_cast<double> (m.req.offset) / static_cast<double> (FRAGMENT_SIZE));
            for(int i = idy; i < num_frags; i++) {
                if(have[idy]) {
                    their_req[i] = true;
                }
            }
        }
    }

    void send_complete_effect () {
        can_send = true;
    }



    bool process_request_precondition () {
        return true;
    }

    void process_request_effect () {
        uint32_t randy = get_random_index ();
        //req.flip (randy);
        fragment retfrag = get_fragment(randy);
        message retm;
        retm.header.message_type = FRAGMENT;
        retm.frag = retfrag;
        sendq.push (retm);

        schedule();
    }

    bool generate_request_precondition () {
        return true;
    }

    message generate_request_effect () {
        message m;
        return m;
    }

    uint32_t get_random_index () {
        //assert (!req.none ());
        uint32_t rf = rand () % their_req.size();
        for (; !their_req[rf]; rf = (rf + 1) % their_req.size ()) { }
        return rf;
    }

    fragment get_fragment (uint32_t idx) {
        fragment f;
        f.offset = (idx * FRAGMENT_SIZE);
        f.fid = m_file.m_fileid;

        if (f.fid.original_length % FRAGMENT_SIZE == 0) {
            memcpy (f.data, m_file.get_data_ptr() + f.offset, FRAGMENT_SIZE);
        }
        else {
            memcpy (f.data, m_file.get_data_ptr() + f.offset, f.fid.original_length % FRAGMENT_SIZE);
        }

        return f;
    }

    UV_UP_INTERNAL (mftp_automaton, process_request);

public:
    mftp_automaton (char* file_name, uint32_t type) :
        m_file(file_name, type),
        have (m_file.m_fileid.hashed_length / FRAGMENT_SIZE),
        their_req (m_file.m_fileid.hashed_length / FRAGMENT_SIZE),
        valid (m_file.m_fileid.hashed_length / FRAGMENT_SIZE),
        can_send (true)
    {
        for(int i = 0; i < have.size(); i++) {
            have[i] = true;
            their_req[i] = false;
            valid[i] = true;
        }
    }

    mftp_automaton (fileID f) :
        have (f.hashed_length / FRAGMENT_SIZE),
        their_req (f.hashed_length / FRAGMENT_SIZE),
        valid (f.hashed_length / FRAGMENT_SIZE),
        can_send (true)
    {
        for(int i = 0; i < have.size(); i++) {
            have[i] = false;
            their_req[i] = false;
            valid[i] = false;
        }
    }

    V_UP_OUTPUT (mftp_automaton, send, message);
    V_UP_INPUT (mftp_automaton, receive, message);
    UV_UP_INPUT (mftp_automaton, send_complete);
};


#endif
