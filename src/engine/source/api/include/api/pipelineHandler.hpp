#ifndef _API_PIPELINE_HANDLER_HPP
#define _API_PIPELINE_HANDLER_HPP

#include <memory>

namespace api
{

template<typename Request, typename Response>
class PipelineHandler
{
protected:
    std::shared_ptr<PipelineHandler> m_next;

    virtual Response impHandle(Request& request, Response&& response) = 0;

public:
    virtual ~PipelineHandler() = default;

    void setNext(std::shared_ptr<PipelineHandler> next) { m_next = next; }

    Response handle(Request& request, Response&& response) { return impHandle(request, std::move(response)); }
};

} // namespace api

#endif // _API_PIPELINE_HANDLER_HPP
